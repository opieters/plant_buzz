#ifndef __ACTUATORS_H__
#define __ACTUATORS_H__

#define N_HEALTH_PIXELS 5
#define N_WATER_PIXELS 5
#define HEALTH_PIXEL_PIN 23
#define WATER_PIXEL_PIN 22

void start_buzzer();
void display_leds_proto(Adafruit_NeoPixel strip, int n_pixels, int amount, int max_amount, uint32_t base_colour);
void display_water_amount(int amount);
void display_health_status(int amount);
void init_actuator();

#endif