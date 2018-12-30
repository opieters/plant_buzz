#include "Dwenguino.h"
#include "SimpleTimer.h"
#include <Wire.h>
#include "OneButton.h"
#include "Adafruit_NeoPixel.h"
#include "math.h"
#include "EEPROM.h"

#define N_PLANTS 16
#define N_HEALTH_PIXELS 5
#define N_WATER_PIXELS 5
#define HEALTH_PIXEL_PIN 20
#define WATER_PIXEL_PIN 20

#define ROM_PLANT_DATA_START 1

// the timer object
SimpleTimer timer;

// EEPROM start address
uint16_t rom_address = ROM_PLANT_DATA_START;


// button objects
OneButton button_up(SW_N, false);
OneButton button_down(SW_S, false);
OneButton button_select(SW_C, false);
OneButton button_left(SW_W, false);
OneButton button_right(SW_E, false);

Adafruit_NeoPixel health_strip = Adafruit_NeoPixel(N_HEALTH_PIXELS, HEALTH_PIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel water_strip = Adafruit_NeoPixel(N_WATER_PIXELS, WATER_PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

typedef struct {short x; short y;} Location; 

typedef struct plant_struct {
  char name[16];
  char latin_name[16];
  long int watering_period;
  long int watering_time;
  short water_amount;
  Location location;
} plant_t;

class Plant{
  private: 
    String name;
    long int watering_period;
    long int watering_time;
    int water_amount;
    Location location;
  public:
    Plant();
    Plant(String name, long int watering_period, int water_amount, Location location);
    long int getWateringTime();
    String getName();
    void update_time(int duration);
};

void save_plant(plant_t* plant, short plant_index){
  int address = ROM_PLANT_DATA_START + plant_index*sizeof(plant_t);
  EEPROM.put(address, *plant); // TODO: use struct here! 
}

void load_plant(plant_t* plant, short plant_index){
  int address = ROM_PLANT_DATA_START + plant_index*sizeof(plant_t);
  EEPROM.get(address, *plant);
}

void init_plant(plant_t* plant){
  int i;
  for(i = 0; i < sizeof(plant->name)/sizeof(plant->name[0]); i++){
    plant->name[i] = NULL;
  }
  for(i = 0; i < sizeof(plant->latin_name)/sizeof(plant->latin_name[0]); i++){
    plant->latin_name[i] = NULL;
  }
  plant->watering_period = 0;
  plant->watering_time = 0;
  plant->water_amount = 0;
}

Plant::Plant(){
  this->name = "";
  this->watering_period = 0;
  this->water_amount = 0;
  this->location = {0, 0};
}

Plant::Plant(String name, long int watering_period, int water_amount, Location location){
  this->name = name;
  this->watering_period = watering_period;
  this->water_amount = water_amount;
  this->location = location;
}

long int Plant::getWateringTime(){
  return watering_time;
}

String Plant::getName(){
  return name;
}

void Plant::update_time(int duration){
  this->watering_time += duration;
}

Plant plants[N_PLANTS];
int n_plants = 0;

// a function to be executed periodically
void minuteCounter() {
  for(int i = 0; i < n_plants; i++){
    plants[i].update_time(-1);
  }
}


void add_plant(Plant p){
  plants[n_plants] = p;
  n_plants++;
}

bool button_clicked = false;
typedef enum {
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_CENTER,
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_NONE=0,
} button_t;
button_t last_pressed_button = BUTTON_NONE;

void click_left(){
  button_clicked = true;
  last_pressed_button = BUTTON_LEFT;
}
void click_right(){
  button_clicked = true;
  last_pressed_button = BUTTON_RIGHT;
}
void click_center(){
  button_clicked = true;
  last_pressed_button = BUTTON_CENTER;
}
void click_up(){
  button_clicked = true;
  last_pressed_button = BUTTON_UP;
}
void click_down(){
  button_clicked = true;
  last_pressed_button = BUTTON_DOWN;
}
void reset_click(){
  button_clicked = false;
  last_pressed_button = BUTTON_NONE;
}


int choice_dialog_screen(String message, String option1="OK", String option2="Cancel"){
  dwenguinoLCD.clear();
  dwenguinoLCD.print(message);
  dwenguinoLCD.setCursor(0, 2);
  String spaces = " ";
  while ((option1.length() + 2 + option2.length() + 2 + spaces.length()) < 16){
    spaces = spaces + " ";
  }
  dwenguinoLCD.print("[" + option1 + "]" + spaces + "[" + option2 + "]");

  reset_click();
  button_left.attachClick(click_left);
  button_right.attachClick(click_right);

  while(!button_clicked){
    button_left.tick();
    button_right.tick();
    timer.run();
  }

  if (last_pressed_button == BUTTON_LEFT){
    return 0;
  } else {
    return 1;
  }
}

int notification_dialogue_screen(String message, String option1="OK"){
  dwenguinoLCD.clear();
  dwenguinoLCD.print(message);
  dwenguinoLCD.setCursor(0, 2);
  option1 = "[" + option1 + "]";
  while ((option1.length() + 2 ) < 16){
    option1 = " " + option1 + " ";
  }
  dwenguinoLCD.print(option1);

  reset_click();
  button_select.attachClick(click_center);

  while(!button_clicked){
    button_select.tick();
    timer.run();
  }
  
  return 0;
}

void start_buzzer(){
  // play a 1kHz sine for 10s
  tone(BUZZER, 1000, 10*1000);
}

void display_leds_proto(Adafruit_NeoPixel strip, int n_pixels, int amount, int max_amount, uint32_t base_colour){
  long int max_level = n_pixels * 255;
  long int level = (long int) (1.0 * amount * max_level / (1.0*max_amount) + 0.5);

  int brightness;
  for(int i = 0; i < N_WATER_PIXELS; i++){
    brightness = (level >> (i*8)) & 0xFF;
    strip.setPixelColor(i, base_colour);
    strip.setBrightness(brightness);
  }
  strip.show();
}

void display_water_amount(int amount){
  display_leds_proto(water_strip, N_WATER_PIXELS, amount, 100, water_strip.Color(0,0,255,0));
}

void display_health_status(int amount){
  display_leds_proto(health_strip, N_HEALTH_PIXELS, amount, 100, water_strip.Color(255,255,0,0));
}

void check_plants(){
  long int lowest_time = 0xFFFFFF;
  int plant_index = -1;
  for(int i = 0; i < n_plants; i++){
    if(plants[i].getWateringTime() < lowest_time){
      lowest_time = plants[i].getWateringTime();
      plant_index = i;
    } 
  }
  if(plant_index != -1){
      dwenguinoLCD.clear();
      dwenguinoLCD.print(plants[plant_index].getName());
      dwenguinoLCD.setCursor(0, 2);
      dwenguinoLCD.print("needs water!");
      start_buzzer();
  } else {
      dwenguinoLCD.clear();
      dwenguinoLCD.print("All plants OK.");
  }
}

int loop_list_screen(String* options, int n_options){
  int cursor_position = 0;
  int cursor = 0;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));
  
  do {
    reset_click();
    button_up.attachClick(click_up);
    button_down.attachClick(click_down);
    button_select.attachClick(click_center);

    while(!button_clicked){
      button_up.tick();
      button_down.tick();
      button_select.tick();
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor_position = 0;
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor_position = 1;
      cursor = min(cursor+1, n_options);
    }

    // update display
    dwenguinoLCD.clear();
    if(cursor_position == 0){
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(options[cursor]);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.print(" ");
      if(n_options > (cursor + 1)){
        dwenguinoLCD.print(options[cursor+1]);
      }
    } else {
      dwenguinoLCD.print(" ");
      if( (cursor-1) > 0){
        dwenguinoLCD.print(options[cursor-1]);
      }
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(options[cursor]);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  return cursor;
}

void setup() {
    initDwenguino();

    Serial.begin(9600);
    timer.setInterval(60000, minuteCounter);
    // add all of the plants over here
    add_plant(Plant("Test", 0, 0, {0, 0}));
    add_plant(Plant("Test", 0, 0, {0, 0}));
    add_plant(Plant("Test", 0, 0, {0, 0}));
    add_plant(Plant("Test", 0, 0, {0, 0}));

    dwenguinoLCD.clear();

    health_strip.begin();
    health_strip.show();
    water_strip.begin();
    water_strip.show();

    noTone(BUZZER);

    byte arrow[8] = {
      B00000,
      B00100,
      B00010,
      B11111,
      B00010,
      B00100,
      B00000,
    };

    dwenguinoLCD.createChar(0, arrow);

    dwenguinoLCD.clear();
    dwenguinoLCD.print("Press C for");
    dwenguinoLCD.print("menu.");

    reset_click();
    button_select.attachClick(click_center);
}

String main_menu_options[] = {String("Summary"), String("View plants"), String("Add plant"), String("Edit plant"), String("About")};
int n_main_menu_options = 5;

void display_long_text(String text){
  int line1_start = 0, line1_stop = 0;
  int line2_start = 0, line2_stop = 0;

  int np = 0, n = 0;
  while((n - line1_start) < 16){
    np = n;
    n = text.indexOf(" ", np+1);
  }
  line1_stop = np;

  np = 0;
  n = 0;
  while((n - line2_start) < 16){
    np = n;
    n = text.indexOf(" ", np+1);
  }
  line1_stop = np;
}

String summary_menu_options[] = {String("Plants that need care soon"), String(""), String(""), String("")};
int n_summary_menu_option = 4;

void summary_menu(){
  int menu_item = loop_list_screen(summary_menu_options, n_summary_menu_option);

  switch(menu_item){
    case 0:
      summary_menu();
      break;
    default:
      
      break;
  }
}

void view_plant_menu(){

}

void add_plant_menu() {

}

void edit_plant_menu(){

}

void about_menu(){
  display_long_text("Plant Buzz. Plant watering system designed for the AIRO office by Olivier Pieters. https://github.com/opieters/plant_buzz");
}

void main_menu(){
  int menu_item = loop_list_screen(main_menu_options, n_main_menu_options);

  switch(menu_item){
    case 0:
      summary_menu();
    case 1:
      view_plant_menu();
    case 2:
      add_plant_menu();
    case 3:
      edit_plant_menu();
    case 4:
      about_menu();
    default:
      break;
  }
}

void loop() {
    timer.run();

    button_select.tick();

    if(button_clicked){
      main_menu();
    }
}
