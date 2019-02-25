#include "plant/plant.h"
#include <stdint.h>
#include "Dwenguino.h"
#include "EEPROM.h"
#include "common/common.h"

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

uint8_t n_plants = 0;

void save_plant(plant_t* plant, short plant_index){
  int address = ROM_PLANT_DATA_START + plant_index*sizeof(plant_t);
  rom_put(address, *plant);
}

void load_plant(plant_t* plant, short plant_index){
  int address = ROM_PLANT_DATA_START + plant_index*sizeof(plant_t);
  rom_get(address, *plant);
}

void init_plant(plant_t* plant){
  unsigned int i;
  for(i = 0; i < (sizeof(plant->name) / sizeof(plant->name[0])); i++){
    plant->name[i] = '\0';
  }
  for(i = 0; i < (sizeof(plant->latin_name)/sizeof(plant->latin_name[0])); i++){
    plant->latin_name[i] = '\0';
  }
  plant->watering_period = 0;
  //plant->watering_time = 0;
  plant->water_amount = 0;
}