#ifndef __USER_H__
#define __USER_H__

typedef struct user_struct {
  char name[16];
  uint16_t n_waterings;
  uint8_t pin[4];
} user_t;

#endif