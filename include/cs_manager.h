#ifndef CS_MANAGER_H
#define CS_MANAGER_H
#include "layout.h"

void cs_select_screen() {
    gpio_put(PIN_CS_SCREEN, 0);
    gpio_put(PIN_CS_SD, 1);
    sleep_us(10);
}

void cs_select_sd() {
    gpio_put(PIN_CS_SD, 0);
    gpio_put(PIN_CS_SCREEN, 1);
    sleep_us(10);
}

void cs_no_select() {
    gpio_put(PIN_CS_SD, 1);
    gpio_put(PIN_CS_SCREEN, 1);
    sleep_us(10);
}


#endif