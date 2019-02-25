#ifndef __UI_H__
#define __UI_H__

#include "menu/menu.h"
#include "Arduino.h"

typedef enum {
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_CENTER,
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_NONE=0,
} button_t;

void click_left();
void click_right();
void click_center();
void click_up();
void click_down();
void reset_click();

void button_up_isr(void);
void button_down_isr(void);
void button_left_isr(void);
void button_right_isr(void);
void button_central_isr(void);
int choice_dialog_screen(String message, String option1="OK", String option2="Cancel");
int notification_dialogue_screen(String message, String option1="OK");
int loop_list_screen(const menu_item_t* options, const int n_options);
int loop_list_message_screen(String message, String* options, int n_options);
void display_long_text(String text);
int select_plant_screen();
void request_text(String message, int max_length, char* user_text);
long int request_number(String message, long int default_number, long int min_number, long int max_number);

void init_ui(void);


void reset_button_input();

void update_button_input();

#endif
