#include "Dwenguino.h"
#include "SimpleTimer.h"
#include <Wire.h>
#include "OneButton.h"
#include "Adafruit_NeoPixel.h"
#include "math.h"
#include <EEPROM.h>
#include <stdint.h>

#include "common/common.h"
#include "ui/ui.h"
#include "user/user.h"
#include "plant/plant.h"
#include "menu/menu.h"
#include "actuator/actuator.h"

#define N_PLANTS 16





// EEPROM start address
uint16_t rom_address = ROM_PLANT_DATA_START;







plant_t plants[N_PLANTS];

int n_users = 0;


// a function to be executed periodically
void minuteCounter() {
  for(int i = 0; i < n_plants; i++){
    //plants[i].watering_time -= -1;
  }
}


void check_plants(){
  long int lowest_time = 0xFFFFFF;
  int plant_index = -1;
  for(int i = 0; i < n_plants; i++){
    /*if(plants[i].watering_time < lowest_time){
      lowest_time = plants[i].watering_time;
      plant_index = i;
    } */
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

void setup() {
    initDwenguino();

    Serial.begin(9600);
    //timer.setInterval(60000, minuteCounter);
    // add all of the plants over here

    dwenguinoLCD.clear();

    init_actuator();
    init_ui();
    
    noTone(BUZZER);

    reset_click();

    // here we add the example
    strncpy(plants[n_plants].latin_name, "Ficus Elastica Abidjan", 16);
    strncpy(plants[n_plants].name, "Alayne", 16);
    plants[n_plants].watering_period = 7;
    plants[n_plants].water_amount = 3;
    plants[n_plants].location = {0, 0};
    //plants[n_plants].watering_time = 0;
    n_plants++;

    strncpy(plants[n_plants].latin_name, "Pachira Aquatica", 16);
    strncpy(plants[n_plants].name, "Quaith", 16);
    plants[n_plants].watering_period = 7;
    plants[n_plants].water_amount = 1;
    plants[n_plants].location = {0, 0};
    //plants[n_plants].watering_time = 0;
    n_plants++;

    strncpy(plants[n_plants].latin_name, "Clusia Rosea Princess", 16);
    strncpy(plants[n_plants].name, "Daemon", 16);
    plants[n_plants].watering_period = 7;
    plants[n_plants].water_amount = 1;
    plants[n_plants].location = {0, 0};
    //plants[n_plants].watering_time = 0;
    n_plants++;

    strncpy(plants[n_plants].latin_name, "Kentia Howea Forsteriana", 16);
    strncpy(plants[n_plants].name, "Egg", 16);
    plants[n_plants].watering_period = 3;
    plants[n_plants].water_amount = 3;
    plants[n_plants].location = {0, 0};
    //plants[n_plants].watering_time = 0;
    n_plants++;

}

String summary_menu_options[] = {String(""), String("Compl. overview"), String("Back"), String("")};
int n_summary_menu_option = 4;


void loop() {
  main_menu();
}
