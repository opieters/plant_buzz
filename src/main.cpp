#include <Arduino.h>
#include <Dwenguino.h>
#include "Adafruit_NeoPixel.h"

#define DISPLAY_LENGTH 16
#define STRIP_PIN 35
#define N_PIXELS 5

int idx, data_byte;
char read_char;
int message_read_status = -1;
short notification_type = 0;
bool buzzer_on;
int plant_id;

char name[DISPLAY_LENGTH+1];
String message;
long int water_amount = 0;
unsigned long notification_update_time;

Adafruit_NeoPixel water_strip(N_PIXELS, STRIP_PIN, NEO_KHZ800 + NEO_BGRW);
Adafruit_NeoPixel notification_strip(N_PIXELS, STRIP_PIN, NEO_KHZ800 + NEO_BGRW);


void setup(void){
    Serial.begin(9600);

    initDwenguino();

    Serial.println("Start");
    
    for(int i = 0; i < (DISPLAY_LENGTH+1); i++){
        name[i] = NULL;
    }

    message = "Press OK";

    dwenguinoLCD.clear();
    dwenguinoLCD.print("All OK.");

    Serial.flush();
    
    water_strip.begin();
    water_strip.show();
    notification_strip.begin();
    notification_strip.show();

    notification_update_time = millis();

    buzzer_on = false;
    plant_id = -1;
}



void loop(){
    if(Serial.available() > 0){
        switch(message_read_status){
            case -1:
                idx = 0;
                message_read_status = 0;
                break;
            case 0:
                data_byte =  Serial.read();
                if(data_byte > 0){
                    read_char = (char) data_byte;
                    if(read_char == ','){
                        message_read_status++;
                        plant_id = 0;
                    } 
                    else {
                        name[idx] = (char) data_byte;
                        idx++;
                        idx = min(idx, DISPLAY_LENGTH); // prevent overflow
                    }
                }
                break;
            case 1:
                data_byte =  Serial.read();
                if(data_byte > 0){
                    read_char = (char) data_byte;
                    if(read_char == ','){
                        message_read_status++;
                    } else {
                        plant_id = plant_id*10 + (read_char - '0');
                    }
                }
                break;
            case 2:
                data_byte =  Serial.read();
                if(data_byte > 0){
                    read_char = (char) data_byte;
                    if(read_char == ','){
                        message_read_status++;
                        water_amount = 0;
                    } else {
                        water_amount = water_amount*10 + (read_char - '0');
                    }
                }
                break;
            case 3:
                data_byte =  Serial.read();
                if(data_byte > 0){
                    notification_type = (char) data_byte - '0';
                    message_read_status++;
                }
                break;
            case 4:
                data_byte =  Serial.read();
                if(data_byte > 0){
                    read_char = (char) data_byte;
                    if(read_char == '\n'){
                        uint8_t color;
                        // update display
                        dwenguinoLCD.clear();
                        dwenguinoLCD.print(name);
                        dwenguinoLCD.setCursor(0, 2);
                        dwenguinoLCD.print(message);
                        

                        // update LED strip
                        for(int i = 0; i < N_PIXELS; i++){
                            if(water_amount > 0){
                                color = min(water_amount, 255);
                            } else {
                                color = 0;
                            }
                            water_strip.setPixelColor(i, 0, 0, color, 0);
                            color -= 255;
                        }
                        water_strip.show();

                        // notification style
                        switch(notification_type){
                            case 0:
                                break;
                            case 1:
                                break;
                            case 2:
                                tone(BUZZER, 1000);
                                break;
                            default:
                                break;
                        }

                        // reset message status
                        message_read_status = -1;
                    }
                }
            default:
                message_read_status = -1;
                break;
        }
    }
    if(notification_type == 2){
        if((millis() - notification_update_time) > 1000){
            notification_update_time = millis();
            if(buzzer_on){
                noTone(BUZZER);
                buzzer_on = false;
            } else {
                tone(BUZZER, 1000);
                buzzer_on = true;
            }
        }
    }
    if(digitalRead(SW_C) == LOW){
        dwenguinoLCD.clear();
        dwenguinoLCD.print("Processing...");

        notification_type = 0;

        for(int i = 0; i < DISPLAY_LENGTH; i++){
            name[i] = ' ';
        }

        noTone(BUZZER);

        notification_strip.clear();
        notification_strip.show();

        // send feedback message
        Serial.println(plant_id, DEC);
        while(digitalRead(SW_C) == LOW);
        delay(500);

        dwenguinoLCD.clear();
        dwenguinoLCD.print("All OK!");
    }
}