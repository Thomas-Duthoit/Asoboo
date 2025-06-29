#ifndef CS_MANAGER_H
#define CS_MANAGER_H
#include "layout.h"
#include "hardware/spi.h"

static inline void cs_select_screen() {
    gpio_put(PIN_CS_SCREEN, 0);
    gpio_put(PIN_CS_SD, 1);
    spi_set_baudrate(SPI_PORT, SPI_BAUDRATE); // Vitesse écran
    sleep_us(10);
}

static inline void cs_select_sd() {
    gpio_put(PIN_CS_SD, 0);
    gpio_put(PIN_CS_SCREEN, 1);
    spi_set_baudrate(SPI_PORT, SPI_SD_READ_BAUDRATE); // Vitesse SD
    sleep_us(10);
}

static inline void cs_select_sd_init() {
    gpio_put(PIN_CS_SD, 0);
    gpio_put(PIN_CS_SCREEN, 1);
    spi_set_baudrate(SPI_PORT, SPI_SD_INIT_BAUDRATE); // Vitesse init SD
    sleep_us(10);
}

static inline void cs_no_select() {
    gpio_put(PIN_CS_SD, 1);
    gpio_put(PIN_CS_SCREEN, 1);
    sleep_us(10);
}


#endif