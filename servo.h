#ifndef SERVO_H
#define SERVO_H
#include "m_curses.h"

void servo_config(void);
void handler_servo(const char signal,struct menu_item *menu_item);
void show_servo(struct menu_item *menu_item);
int servo_get_level(void);
void servo_level(int level);
#endif
