#ifndef  DRIVER_BTN_H
#define  DRIVER_BTN_H

#include "pico/stdlib.h"

char get_btn_state(uint32_t button);



char get_btn_state(uint32_t button) { return !gpio_get(button); }

#endif