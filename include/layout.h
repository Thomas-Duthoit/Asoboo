#ifndef LAYOUT_H
#define LAYOUT_H
#include "pico/stdlib.h"


// All the hardware connections and baudrates are specified here


// BUTTONS
#define PIN_BTN_A     14
#define PIN_BTN_B     13
#define PIN_BTN_UP    11
#define PIN_BTN_DOWN  9
#define PIN_BTN_LEFT  10
#define PIN_BTN_RIGHT 12
#define PIN_BTN_HOME  8

// SPI
#define PIN_MISO      16
#define PIN_MOSI      19
#define PIN_SCK       18

#define PIN_CS_SCREEN 17
#define PIN_CS_SD 26

#define SPI_PORT     spi0
#define SPI_BAUDRATE         62.5 * 1000 * 1000       // 62.5 Mhz
#define SPI_SD_INIT_BAUDRATE 100 * 1000               // 100 kHz
#define SPI_SD_READ_BAUDRATE 5 * 1000 * 1000          // 5 MHz


// SCREEN
#define PIN_DC   20
#define PIN_RST  21
#define PIN_BL   22

// BUZZER
#define PIN_BUZ  15

// STATUS LED
#define PIN_LED  25

#endif