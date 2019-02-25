#include "menu/menu.h"
#include "Arduino.h"
#include "ui/ui.h"
#include "Dwenguino.h"
#include "plant/plant.h"

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

void view_plant_menu(){
  
}



#define MAX_PLANT_S_LENGTH 16
void edit_plant_menu(plant_t* plant) {
  request_text("Plant name:", MAX_PLANT_S_LENGTH, plant->name);
  request_text("Latin name:", MAX_PLANT_S_LENGTH, plant->latin_name);
  plant->water_amount = (short) request_number("Amount of water:", plant->water_amount, 1, 5);
  plant->watering_period = request_number("Watering period (hrs):", plant->watering_period, 1, 31*24*12);
  plant->location.x = (short) request_number("X location:", plant->location.x, 0, 8);
  plant->location.y = (short) request_number("Y location:", plant->location.y, 0, 8);
}

void add_plant_menu(){
  plant_t p;
  uint8_t i;

  p.water_amount = 1;
  p.watering_period = 24;
  p.location = {0, 0};
  //p.watering_time = 0;
  
  for(i = 0; i < 16; i++){
    p.name[i] = '\0';
    p.latin_name[i] = '\0';
  }

  edit_plant_menu(&p);

  save_plant(&p, n_plants);
  n_plants++;
}

void edit_existing_plant_menu(){
  plant_t p;
  int selected_plant = select_plant_screen();
  load_plant(&p, selected_plant);
  edit_plant_menu(&p);
}

void about_menu(){
  display_long_text("Plant Buzz. Plant watering system designed for the AIRO office by Olivier Pieters. https://github.com/opieters/plant_buzz");
}

void main_menu(){
  int i;
  bool all_ok = true;
  plant_t p;
  uint8_t watering_plant_index = 0;
  float pct_overtime = 0.0;

  for(i = 0; i < n_plants; i++){
    load_plant(&p, i);
    if((millis() - p.last_watering_time) > p.watering_period){
      float pct = 1.0*(millis() - p.last_watering_time) / p.watering_period;
      all_ok = false;
      if(pct > pct_overtime){
        pct_overtime = pct;
        watering_plant_index = i;
      }
    }
  }
  if(all_ok){
    dwenguinoLCD.clear();
    dwenguinoLCD.print("All OK");
  } else {
    load_plant(&p, watering_plant_index);
    dwenguinoLCD.clear();
    dwenguinoLCD.print(p.name);
    dwenguinoLCD.setCursor(0, 2);
    dwenguinoLCD.print("needs water.");
  }
}