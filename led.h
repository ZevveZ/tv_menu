#ifndef LED_H
#define LED_H

#include "m_curses.h"

void led_config(void);
void handler_led(const char signal, struct menu_item *menu_item);
void led_dark(void);

#endif
