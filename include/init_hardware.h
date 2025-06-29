#ifndef INIT_HARDWARE_HARDWARE_H
#define INIT_HARDWARE_HARDWARE_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "layout.h"
#include "driver_screen.h"
#include "driver_btn.h"
#include "hardware/watchdog.h"




void init_hardware() {
    // Initialisation des boutons
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

    spi_init(SPI_PORT, SPI_BAUDRATE);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_SCREEN);
    gpio_set_dir(PIN_CS_SCREEN, GPIO_OUT);
    gpio_put(PIN_CS_SCREEN, 1); // Désactiver l'écran par défaut
    gpio_init(PIN_CS_SD);
    gpio_set_dir(PIN_CS_SD, GPIO_OUT);
    gpio_put(PIN_CS_SD, 1); // Désactiver la carte SD par défaut

    // Initialisation de l'écran
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_init(PIN_BL);
    gpio_set_dir(PIN_BL, GPIO_OUT);
    
    // Initialisation du buzzer
    gpio_init(PIN_BUZ);
    gpio_set_dir(PIN_BUZ, GPIO_OUT);

    // Initialisation de la led
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

    // Ecran
    init_screen();

    // Reset + Allumage
    // uint16_t img[SCREEN_WIDTH*SCREEN_HEIGHT] = _BOOT_IMAGE;
    // g_draw_buffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img);
    // _render_boot_img();
    screen_backlight(1);


    // uSD
    // if (!init_fatfs()) {
    //     spi_init(SPI_PORT, SPI_BAUDRATE);
    //     // g_fill_rect(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, RED);
    //     // g_fill_rect(2, 2, SCREEN_WIDTH-3, SCREEN_HEIGHT-3, BLACK);
    //     g_draw_string(5, 5, "ERROR:", RED, BLACK);
    //     g_draw_string(5+FONT_WIDTH*7, 5, "NO SD CARD", WHITE, BLACK);
    //     g_draw_string(5, 5+FONT_HEIGHT, "<HOME> TO RETRY", WHITE, BLACK);

    //     while (1) {
    //         tight_loop_contents();

    //         if (get_btn_state(PIN_BTN_HOME)) watchdog_reboot(0, 0, 0);
    //     }
    // }
}
#endif
