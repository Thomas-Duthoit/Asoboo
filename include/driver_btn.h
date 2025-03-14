#ifndef  DRIVER_BTN_H
#define  DRIVER_BTN_H

#include "pico/stdlib.h"

bool get_btn_state(uint32_t button);



bool get_btn_state(uint32_t button) { return !gpio_get(button); }

#endif