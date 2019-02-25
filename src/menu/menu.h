#ifndef __MENU_H__
#define __MENU_H__

#include "plant/plant.h"

typedef struct {
  const char* text;
  const void (*callback) (void);
} menu_item_t;

void plant_care_menu();
void summary_menu();
void view_plant_menu();

void edit_plant_menu(plant_t* plant);
void add_plant_menu();
void edit_existing_plant_menu();
void about_menu();
void main_menu();

#endif 
