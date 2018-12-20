#include "Dwenguino.h"
#include "SimpleTimer.h"
#include <Wire.h>
#include "OneButton.h"
#include "Adafruit_NeoPixel.h"

#define N_PLANTS 16
#define N_HEALTH_PIXELS 5
#define N_WATER_PIXELS 5
#define HEALTH_PIXEL_PIN 20
#define WATER_PIXEL_PIN 20

// the timer object
SimpleTimer timer;

// button objects
OneButton button_up(SW_N, false);
OneButton button_down(SW_S, false);
OneButton button_select(SW_C, false);
OneButton button_left(SW_W, false);
OneButton button_right(SW_E, false);

Adafruit_NeoPixel health_strip = Adafruit_NeoPixel(N_HEALTH_PIXELS, HEALTH_PIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel water_strip = Adafruit_NeoPixel(N_WATER_PIXELS, WATER_PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

typedef struct {int x; int y;} Location; 

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
};

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

Plant plants[N_PLANTS];

// a function to be executed periodically
void minuteCounter() {
    Serial.print("Uptime (s): ");
    Serial.println(millis() / 1000);
}

int n_plants = 0;
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
  }
  
  return 0;
}

void start_buzzer(){
  // play a 1kHz sine for 10s
  tone(BUZZER, 1000, 10*1000);
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

void setup() {
    Serial.begin(9600);
    timer.setInterval(1000*60, minuteCounter);
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
}

void loop() {
    timer.run();
}