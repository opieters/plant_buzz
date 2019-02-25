#ifndef __PLANT_H__
#define __PLANT_H__

#include "common/common.h"
#include "Arduino.h"

#define MAX_PLANT_S_LENGTH 16
#define ROM_PLANT_DATA_START 1

typedef struct plant_struct {
  char name[16];
  char latin_name[16];
  unsigned long int watering_period;
  unsigned long int last_watering_time;
  short water_amount;
  Location location;
} plant_t;

void save_plant(plant_t* plant, short plant_index);
void load_plant(plant_t* plant, short plant_index);
void init_plant(plant_t* plant);

extern uint8_t n_plants;

#endif