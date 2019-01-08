#include "Dwenguino.h"
#include "SimpleTimer.h"
#include <Wire.h>
#include "OneButton.h"
#include "Adafruit_NeoPixel.h"
#include "math.h"
#include <EEPROM.h>
#include <stdint.h>

#define N_PLANTS 16
#define N_HEALTH_PIXELS 5
#define N_WATER_PIXELS 5
#define HEALTH_PIXEL_PIN 23
#define WATER_PIXEL_PIN 22

#define ROM_PLANT_DATA_START 1

// the timer object
SimpleTimer timer;

// EEPROM start address
uint16_t rom_address = ROM_PLANT_DATA_START;


// button objects
OneButton button_up(SW_N, true);
OneButton button_down(SW_S, true);
OneButton button_select(SW_C, true);
OneButton button_left(SW_W, true);
OneButton button_right(SW_E, true);

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

typedef struct {
  const char* text;
  const void (*callback) (void);
} menu_item_t;

plant_t plants[N_PLANTS];
int n_plants = 0;

template< typename T > T& rom_get( int idx, T &t ){
    uint8_t *ptr = (uint8_t*) &t;
    for( int count = sizeof(T) ; count > 0 ; count-- ){  
      *ptr = EEPROM.read(idx);
      ptr++;
      idx++;
    }
    return t;
}

template< typename T > const T& rom_put( int idx, const T &t ){
    const uint8_t *ptr = (const uint8_t*) &t;
    for( int count = sizeof(T) ; count ; count-- ){
      EEPROM.write(idx, *ptr);
      ptr++;
      idx++;
    }  
    return t;
}

void save_plant(plant_t* plant, short plant_index){
  int address = ROM_PLANT_DATA_START + plant_index*sizeof(plant_t);
  rom_put(address, *plant);
}

void load_plant(plant_t* plant, short plant_index){
  int address = ROM_PLANT_DATA_START + plant_index*sizeof(plant_t);
  rom_get(address, *plant);
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

// a function to be executed periodically
void minuteCounter() {
  for(int i = 0; i < n_plants; i++){
    plants[i].watering_time -= -1;
  }
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
    if(plants[i].watering_time < lowest_time){
      lowest_time = plants[i].watering_time;
      plant_index = i;
    } 
  }
  if(plant_index != -1){
      dwenguinoLCD.clear();
      dwenguinoLCD.print(plants[plant_index].name);
      dwenguinoLCD.setCursor(0, 2);
      dwenguinoLCD.print("needs water!");
      start_buzzer();
  } else {
      dwenguinoLCD.clear();
      dwenguinoLCD.print("All plants OK.");
  }
}

int loop_list_screen(menu_item_t* options, int n_options){
  int cursor = 0;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));
  
  do {
    reset_click();
    button_up.attachClick(click_up);
    button_down.attachClick(click_down);
    button_select.attachClick(click_center);

    // update display
    dwenguinoLCD.clear();
    if (cursor == (n_options-1)){
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(options[cursor-1].text);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(options[cursor].text);
    }  else {
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(options[cursor].text);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(options[cursor+1].text);
    }

    while(!button_clicked){
      button_up.tick();
      button_down.tick();
      button_select.tick();
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor = min(cursor+1, n_options-1);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  return cursor;
}

int loop_list_message_screen(String message, String* options, int n_options){
  int cursor = 0;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));
  
  do {
    reset_click();
    button_up.attachClick(click_up);
    button_down.attachClick(click_down);
    button_select.attachClick(click_center);

    // update display
    dwenguinoLCD.clear();
    dwenguinoLCD.print(message);
    dwenguinoLCD.setCursor(0,2);
    dwenguinoLCD.print(byte(0));
    dwenguinoLCD.print(options[cursor+1]);

    while(!button_clicked){
      button_up.tick();
      button_down.tick();
      button_select.tick();
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor = min(cursor+1, n_options-1);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  return cursor;
}

void setup() {
    initDwenguino();

    Serial.begin(9600);
    timer.setInterval(60000, minuteCounter);
    // add all of the plants over here

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
    dwenguinoLCD.setCursor(0, 2);
    dwenguinoLCD.print("menu.");

    reset_click();
    button_select.attachClick(click_center);

    strncpy(plants[n_plants].latin_name, "Ficus Elastica Abidjan", 16);
    strncpy(plants[n_plants].name, "Alayne", 16);
    plants[n_plants].watering_period = 7;
    plants[n_plants].water_amount = 3;
    plants[n_plants].location = {0, 0};
    plants[n_plants].watering_time = 0;
    n_plants++;

    strncpy(plants[n_plants].latin_name, "Pachira Aquatica", 16);
    strncpy(plants[n_plants].name, "Quaith", 16);
    plants[n_plants].watering_period = 7;
    plants[n_plants].water_amount = 1;
    plants[n_plants].location = {0, 0};
    plants[n_plants].watering_time = 0;
    n_plants++;

    strncpy(plants[n_plants].latin_name, "Clusia Rosea Princess", 16);
    strncpy(plants[n_plants].name, "Daemon", 16);
    plants[n_plants].watering_period = 7;
    plants[n_plants].water_amount = 1;
    plants[n_plants].location = {0, 0};
    plants[n_plants].watering_time = 0;
    n_plants++;

    strncpy(plants[n_plants].latin_name, "Kentia Howea Forsteriana", 16);
    strncpy(plants[n_plants].name, "Egg", 16);
    plants[n_plants].watering_period = 3;
    plants[n_plants].water_amount = 3;
    plants[n_plants].location = {0, 0};
    plants[n_plants].watering_time = 0;
    n_plants++;

}

const menu_item_t mm_plant_care = {.text = "Plant care", .callback = NULL};
const menu_item_t mm_view_plants = {.text = "Summary", .callback = NULL};
const menu_item_t mm_add_plant = {.text = "View plants", .callback = NULL};
const menu_item_t mm_view_plant = {.text = "Add plant", .callback = NULL};
const menu_item_t mm_edit_plant = {.text = "Edit plant", .callback = NULL};
const menu_item_t mm_about = {.text = "About", .callback = NULL};

const menu_item_t main_menu_options[] = {
  mm_plant_care,
  mm_view_plants,
  mm_add_plant,
  mm_view_plant,
  mm_edit_plant,
  mm_about
};

int n_main_menu_options = 6;

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

String summary_menu_options[] = {String(""), String("Compl. overview"), String("Back"), String("")};
int n_summary_menu_option = 4;

void plant_care_menu(){
 // TODO: list all plants that need care here
  String plant_care_list[] = {String("back")};
  int plant_idx = loop_list_message_screen("Care needed for:", plant_care_list, sizeof(plant_care_list) /sizeof(plant_care_list[0]));
  switch(plant_idx){
    case 1:
      // TODO: display plant status
    default:
      return;
  }
}

void summary_menu(){
  int menu_item = 00;//loop_list_screen(summary_menu_options, n_summary_menu_option);

  switch(menu_item){
    case 0:
      summary_menu();
      break;
    default:
      
      break;
  }
}

int select_plant_screen(){
  int cursor = 0;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));
  
  do {
    reset_click();
    button_up.attachClick(click_up);
    button_down.attachClick(click_down);
    button_select.attachClick(click_center);

    // update display
    dwenguinoLCD.clear();
    if (cursor == (n_plants-1)){
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(plants[cursor-1].name);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(plants[cursor].name);
    }  else {
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(plants[cursor].name);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(plants[cursor+1].name);
    }

    while(!button_clicked){
      button_up.tick();
      button_down.tick();
      button_select.tick();
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor = min(cursor+1, n_plants-1);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  return cursor;
}

void view_plant_menu(){
  
}

void request_text(String message, int max_length, char* user_text){
  int cursor = 0;
  dwenguinoLCD.cursor();
  do {
    reset_click();
    button_up.attachClick(click_up);
    button_down.attachClick(click_down);
    button_left.attachClick(click_left);
    button_right.attachClick(click_right);
    button_select.attachClick(click_center);

    // update display
    dwenguinoLCD.clear();
    dwenguinoLCD.print(message);
    dwenguinoLCD.setCursor(0, 2);
    dwenguinoLCD.print(user_text);
    dwenguinoLCD.setCursor(cursor, 2);

    while(!button_clicked){
      button_up.tick();
      button_down.tick();
      button_left.tick();
      button_right.tick();
      button_select.tick();
      timer.run();
    }

    switch(last_pressed_button){
      case BUTTON_UP:
        user_text[cursor] = max(user_text[cursor]+1, 'A');
        break;
      case BUTTON_DOWN:
        user_text[cursor] = min(user_text[cursor]-1, 'z');
        break;
      case BUTTON_LEFT:
        cursor = max(cursor-1, 0);
        break;
      case BUTTON_RIGHT:
        cursor = min(cursor+1, 15);
        break;
      default:
        break;
      }
  } while(last_pressed_button != BUTTON_CENTER);

  dwenguinoLCD.noCursor();
}

long int request_number(String message, long int default_number, long int min_number, long int max_number){
  int step = 1;

  do {
    reset_click();
    button_up.attachClick(click_up);
    button_down.attachClick(click_down);
    button_left.attachClick(click_left);
    button_right.attachClick(click_right);
    button_select.attachClick(click_center);

    // update display
    dwenguinoLCD.clear();
    dwenguinoLCD.print(message);
    dwenguinoLCD.setCursor(0, 2);
    dwenguinoLCD.print(default_number);

    while(!button_clicked){
      button_up.tick();
      button_down.tick();
      button_left.tick();
      button_right.tick();
      button_select.tick();
      timer.run();
    }

    switch(last_pressed_button){
      case BUTTON_UP:
        default_number = min(default_number + step, max_number);
        break;
      case BUTTON_DOWN:
        default_number = max(default_number - step, min_number);
        break;
      case BUTTON_LEFT:
        step = max(step / 10, 1);
        break;
      case BUTTON_RIGHT:
        step = step * 10;
        break;
      default:
        break;
      }
  } while(last_pressed_button != BUTTON_CENTER);
  return default_number;
}

#define MAX_PLANT_S_LENGTH 16
void add_plant_menu(const int plant_idx = -1) {
  plant_t* plant;

  if(plant_idx == -1){
    plant = &plants[n_plants];
  } else {
    plant = &plants[plant_idx];
  }

  request_text("Plant name:", MAX_PLANT_S_LENGTH, plant->name);
  request_text("Latin name:", MAX_PLANT_S_LENGTH, plant->latin_name);
  plant->water_amount = (short) request_number("Amount of water:", 1, 1, 5);
  plant->watering_period = request_number("Watering period (hrs):", 24, 1, 31*24*12);
  plant->location.x = (short) request_number("X location:", 0, 0, 8);
  plant->location.y = (short) request_number("Y location:", 0, 0, 8);

  if(plant_idx == -1){
      n_plants++;
  }
}

void edit_plant_menu(){
  int selected_plant = select_plant_screen();
}

void about_menu(){
  display_long_text("Plant Buzz. Plant watering system designed for the AIRO office by Olivier Pieters. https://github.com/opieters/plant_buzz");
}

void main_menu(){
  int menu_item = loop_list_screen(main_menu_options, n_main_menu_options);

  switch(menu_item){
    case 0:
      plant_care_menu();
    case 1:
      summary_menu();
    case 2:
      view_plant_menu();
    case 3:
      add_plant_menu();
    case 4:
      edit_plant_menu();
    case 5:
      about_menu();
    default:
      break;
  }
}

void loop() {
  main_menu();
}
