#ifndef INIT_HARDWARE_HARDWARE_H
#define INIT_HARDWARE_HARDWARE_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "layout.h"
#include "driver_screen.h"
#include "driver_btn.h"
#include "hardware/watchdog.h"




void init_hardware() {
    // BUTTONS HW INIT (pulled up)
    gpio_init(PIN_BTN_A);
    gpio_init(PIN_BTN_B);
    gpio_init(PIN_BTN_UP);
    gpio_init(PIN_BTN_DOWN);
    gpio_init(PIN_BTN_LEFT);
    gpio_init(PIN_BTN_RIGHT);
    gpio_init(PIN_BTN_HOME);

    gpio_set_dir(PIN_BTN_A, GPIO_IN);
    gpio_set_dir(PIN_BTN_B, GPIO_IN);
    gpio_set_dir(PIN_BTN_UP, GPIO_IN);
    gpio_set_dir(PIN_BTN_DOWN, GPIO_IN);
    gpio_set_dir(PIN_BTN_LEFT, GPIO_IN);
    gpio_set_dir(PIN_BTN_RIGHT, GPIO_IN);
    gpio_set_dir(PIN_BTN_HOME, GPIO_IN);
    
    gpio_pull_up(PIN_BTN_A);
    gpio_pull_up(PIN_BTN_B);
    gpio_pull_up(PIN_BTN_UP);
    gpio_pull_up(PIN_BTN_DOWN);
    gpio_pull_up(PIN_BTN_LEFT);
    gpio_pull_up(PIN_BTN_RIGHT);
    gpio_pull_up(PIN_BTN_HOME);

    // SPI BUS AND CS HW INIT
    spi_init(SPI_PORT, SPI_BAUDRATE);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_SCREEN);
    gpio_set_dir(PIN_CS_SCREEN, GPIO_OUT);
    gpio_put(PIN_CS_SCREEN, 1); // Screen is disabled
    gpio_init(PIN_CS_SD);
    gpio_set_dir(PIN_CS_SD, GPIO_OUT);
    gpio_put(PIN_CS_SD, 1); // uSD is disabled

    // SCREEN HW INIT
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_init(PIN_BL);
    gpio_set_dir(PIN_BL, GPIO_OUT);
    
    // BUZZER HW INIT
    gpio_init(PIN_BUZ);
    gpio_set_dir(PIN_BUZ, GPIO_OUT);

    // LED HW INIT
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

    // SCREEN
    init_screen();
    screen_backlight(1);

}
#endif
