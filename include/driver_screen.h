#ifndef DRIVER_SCREEN_H
#define DRIVER_SCREEN_H

#include "layout.h"
#include "hardware/spi.h"
#include "cs_manager.h"

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 128
#define SCREEN_X_OFF 1
#define SCREEN_Y_OFF 2

// ST7735 commands
#define CMD_SWRESET  0x01
#define CMD_SLPOUT   0x11
#define CMD_DISPOFF  0x28
#define CMD_DISPON   0x29
#define CMD_CASET    0x2A
#define CMD_RASET    0x2B
#define CMD_RAMWR    0x2C
#define CMD_MADCTL   0x36
#define CMD_COLMOD   0x3A


bool init_screen();
void screen_backlight(bool backlight);
void screen_write_command(uint8_t cmd);
void screen_write_data(uint8_t *data, size_t len);



void screen_write_command(uint8_t cmd) {
    gpio_put(PIN_DC, 0);  // DC low for command
    cs_select_screen();
    spi_write_blocking(SPI_PORT, &cmd, 1);
    cs_no_select();
}

void screen_write_data(uint8_t *data, size_t len) {
    gpio_put(PIN_DC, 1);  // DC high for data
    cs_select_screen();
    spi_write_blocking(SPI_PORT, data, len);
    cs_no_select();
}

void screen_backlight(bool backlight) {
    gpio_put(PIN_BL, backlight);
}



bool init_screen() {
    uint8_t data[4];

    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);
    sleep_ms(150);
    
    screen_write_command(CMD_SWRESET);
    sleep_ms(150);
    screen_write_command(CMD_SLPOUT);
    sleep_ms(500);

    // Frame Rate Control
    screen_write_command(0xB1);
    data[0] = 0x01;
    data[1] = 0x2C;
    data[2] = 0x2D;
    screen_write_data(data, 3);

    // Display inversion control
    screen_write_command(0xB4);
    screen_write_data(&(uint8_t){0x07}, 1);

    // Power control
    screen_write_command(0xC0);
    data[0] = 0xA2;
    data[1] = 0x02;
    data[2] = 0x84;
    screen_write_data(data, 3);

    screen_write_command(0xC1);
    screen_write_data(&(uint8_t){0xC5}, 1);

    screen_write_command(0xC2);
    data[0] = 0x0A;
    data[1] = 0x00;
    screen_write_data(data, 2);

    screen_write_command(0xC3);
    data[0] = 0x8A;
    data[1] = 0x2A;
    screen_write_data(data, 2);

    screen_write_command(0xC4);
    data[0] = 0x8A;
    data[1] = 0xEE;
    screen_write_data(data, 2);

    screen_write_command(0xC5);  // VCOM Control
    screen_write_data(&(uint8_t){0x0E}, 1);

    // Memory Data Access Control
    screen_write_command(CMD_MADCTL);
    screen_write_data(&(uint8_t){0b10100000}, 1);  // MX | MV

    // Pixel Format Set
    screen_write_command(CMD_COLMOD);
    screen_write_data(&(uint8_t){0x05}, 1);  // 16-bit color

    // Turn on the display
    screen_write_command(CMD_DISPON);
    sleep_ms(1500);
}
#endif