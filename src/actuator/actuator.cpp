#include "Adafruit_NeoPixel.h"
#include "actuator/actuator.h"

Adafruit_NeoPixel health_strip = Adafruit_NeoPixel(N_HEALTH_PIXELS, HEALTH_PIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel water_strip = Adafruit_NeoPixel(N_WATER_PIXELS, WATER_PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

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

void init_actuator(void){
    health_strip.begin();
    health_strip.show();
    water_strip.begin();
    water_strip.show();
}