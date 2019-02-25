#include "ui/ui.h"
#include "Dwenguino.h"
#include "SimpleTimer.h"
#include "DwenguinoLCD.h"
#include "OneButton.h"
#include "plant/plant.h"

// the timer object
SimpleTimer timer;

volatile bool button_clicked = false;

volatile button_t last_pressed_button = BUTTON_NONE;

volatile uint8_t button_left_enabled = 0;
volatile uint8_t button_right_enabled = 0;
volatile uint8_t button_center_enabled = 0;
volatile uint8_t button_up_enabled = 0;
volatile uint8_t button_down_enabled = 0;
uint8_t button_mask = 0x0;


// button objects

ISR(TIMER3_CAPT_vect){
  button_clicked = true;
  last_pressed_button = BUTTON_CENTER;
}

void button_up_isr(void){
  button_clicked = true;
  last_pressed_button = BUTTON_UP;
}
void button_down_isr(void){
  button_clicked = true;
  last_pressed_button = BUTTON_DOWN;
}
void button_left_isr(void){
  button_clicked = true;
  last_pressed_button = BUTTON_LEFT;
}
void button_right_isr(void){
  button_clicked = true;
  last_pressed_button = BUTTON_RIGHT;
}
void button_central_isr(void){
  button_clicked = true;
  last_pressed_button = BUTTON_CENTER;
}

void init_ui(){
  // enable pull-up on pin (TODO; needed?)
  EICRB = 0b10101010; // configure falling edge
  EIMSK = 0b11110000; // mask
  //PCICR = 0b1; // enable pin change interrupt
  PCMSK0 = 0b0000000;
  // enable interrupt

  // configure time to filter double clicks
  TCCR1A = 0; // no OC or WGM
  TIMSK1 = _BV(TOIE1); // timer overflow interrupt enabled
  TIFR1 = _BV(TOV1); // clear overflow flag
  OCR1A = 0xAF; // 1ms interrupt
  TCNT1 = 0; // clear timer count register
  TCCR1B = 0b00001000; // CTC + disable 

  attachInterrupt(0, button_up_isr, FALLING);
  attachInterrupt(0, button_down_isr, FALLING);
  attachInterrupt(0, button_central_isr, FALLING);
  attachInterrupt(0, button_left_isr, FALLING);
  attachInterrupt(0, button_right_isr, FALLING);

    byte arrow[8] = {
      B00000,
      B00100,
      B00010,
      B11111,
      B00010,
      B00100,
      B00000,
    };

    dwenguinoLCD.createChar(0, arrow);

}

uint8_t led_status = 0;
ISR(TIMER1_COMPA_vect){
  EIMSK = button_mask;
  TCCR1B = 0; // disable
}

void start_button_timer(){
   TCCR1B = 0b10001101; // IC noise canceling + falling edge + CTC + 1:1024 prescaler
}

void reset_button_input(){
  button_left_enabled = 0;
  button_right_enabled = 0;
  button_center_enabled = 0;
  button_up_enabled = 0;
  button_down_enabled = 0;

  update_button_input();
}

void update_button_input(){
  if(button_left_enabled){
    button_mask |= 0b00010000;
  } else {
    button_mask &= 0b11101111;
  }
  if(button_right_enabled){
    button_mask |= 0b01000000;
  } else {
    button_mask &= 0b10111111;
  }
  if(button_up_enabled){
    button_mask |= 0b10000000;
  } else {
    button_mask &= 0b01111111;
  }
  if(button_down_enabled){
    button_mask |= 0b00100000;
  } else {
    button_mask &= 0b11011111;
  }
  EIMSK = button_mask;
}

// enable external interrupts
//; 
//ISC70
/*
ISR(INT4_vect) {
  EIMSK &= 0b11101111;
  button_clicked = true;
  last_pressed_button = BUTTON_LEFT;
  start_button_timer();
}
ISR(INT5_vect) {
  EIMSK &= 0b11011111;
  button_clicked = true;
  last_pressed_button = BUTTON_DOWN;
  start_button_timer();
}
ISR(INT6_vect) {
  EIMSK &= 0b10111111;
  button_clicked = true;
  last_pressed_button = BUTTON_RIGHT;
  start_button_timer();
  
}
ISR(INT7_vect) {
  EIMSK &= 0b01111111;
  button_clicked = true;
  last_pressed_button = BUTTON_UP;
  start_button_timer();
}*/

inline void reset_click(){
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
  while(!button_clicked){
    timer.run();
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
  while(!button_clicked){
    timer.run();
  }
  
  return 0;
}


int loop_list_screen(const menu_item_t* options, const int n_options){
  int cursor = 0;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));

  button_left_enabled = 0;
  button_right_enabled = 0;
  button_center_enabled = 1;
  button_up_enabled = 1;
  button_down_enabled = 1;
  update_button_input();
  
  do {
    // update display
    dwenguinoLCD.clear();
    if (cursor == (n_options-1)){
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(options[cursor-1].text);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(options[cursor].text);
    }  else {
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(options[cursor].text);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(options[cursor+1].text);
    }

    reset_click();
    while(!button_clicked){
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor = min(cursor+1, n_options-1);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  reset_button_input();

  return cursor;
}

int loop_list_message_screen(String message, String* options, int n_options){
  int cursor = 0;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));
  
  do {
    // update display
    dwenguinoLCD.clear();
    dwenguinoLCD.print(message);
    dwenguinoLCD.setCursor(0,2);
    dwenguinoLCD.print(byte(0));
    dwenguinoLCD.print(options[cursor+1]);

    reset_click();
    while(!button_clicked){
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor = min(cursor+1, n_options-1);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  return cursor;
}

void display_long_text(String text){
  int line1_start = 0, line1_stop = 0;
  int line2_start = 0, line2_stop = 0;

  int np = 0, n = 0;
  while((n - line1_start) < 16){
    np = n;
    n = text.indexOf(" ", np+1);
  }
  line1_stop = np;

  np = 0;
  n = 0;
  while((n - line2_start) < 16){
    np = n;
    n = text.indexOf(" ", np+1);
  }
  line1_stop = np;
}

int select_plant_screen(){
  int cursor = 0;
  plant_t p1, p2;
  
  dwenguinoLCD.clear();
  dwenguinoLCD.write(byte(0));
  
  do {

    // update display
    dwenguinoLCD.clear();
    if (cursor == (n_plants-1)){
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(p1.name);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(p2.name);
    }  else {
      dwenguinoLCD.write(byte(0));
      dwenguinoLCD.print(p1.name);
      dwenguinoLCD.setCursor(0,2);
      dwenguinoLCD.print(" ");
      dwenguinoLCD.print(p2.name);
    }

    reset_click();
    while(!button_clicked){
      timer.run();
    }
    // update cursur position
    if(last_pressed_button == BUTTON_UP){
      cursor = max(cursor-1, 0);
    }
    if(last_pressed_button == BUTTON_DOWN){
      cursor = min(cursor+1, n_plants-1);
    }
  } while(last_pressed_button != BUTTON_CENTER);

  return cursor;
}


void request_text(String message, int max_length, char* user_text){
  int cursor = 0;
  dwenguinoLCD.cursor();
  do {
    // update display
    dwenguinoLCD.clear();
    dwenguinoLCD.print(message);
    dwenguinoLCD.setCursor(0, 2);
    dwenguinoLCD.print(user_text);
    dwenguinoLCD.setCursor(cursor, 2);

    reset_click();
    while(!button_clicked){
      timer.run();
    }

    switch(last_pressed_button){
      case BUTTON_UP:
        user_text[cursor] = max(user_text[cursor]+1, 'A');
        break;
      case BUTTON_DOWN:
        user_text[cursor] = min(user_text[cursor]-1, 'z');
        break;
      case BUTTON_LEFT:
        cursor = max(cursor-1, 0);
        break;
      case BUTTON_RIGHT:
        cursor = min(cursor+1, 15);
        break;
      default:
        break;
      }
  } while(last_pressed_button != BUTTON_CENTER);

  dwenguinoLCD.noCursor();
}

long int request_number(String message, long int default_number, long int min_number, long int max_number){
  int step = 1;

  do {
    // update display
    dwenguinoLCD.clear();
    dwenguinoLCD.print(message);
    dwenguinoLCD.setCursor(0, 2);
    dwenguinoLCD.print(default_number);

    reset_click();
    while(!button_clicked){
      timer.run();
    }

    switch(last_pressed_button){
      case BUTTON_UP:
        default_number = min(default_number + step, max_number);
        break;
      case BUTTON_DOWN:
        default_number = max(default_number - step, min_number);
        break;
      case BUTTON_LEFT:
        step = max(step / 10, 1);
        break;
      case BUTTON_RIGHT:
        step = step * 10;
        break;
      default:
        break;
      }
  } while(last_pressed_button != BUTTON_CENTER);
  return default_number;
}