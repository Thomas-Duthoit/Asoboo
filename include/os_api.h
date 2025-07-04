#ifndef OS_API_H
#define OS_API_H


/*
Include this file when creating a game for Asoboo to access hardware level function
*/


// GAME TEMPLATE
/*
#include "os_api.h"  // include this file

__attribute__((used)) __attribute__((section(".text.game_main"))) // USED TO BE KEPT BY THE COMPILER
void game_main(os_api_t* api) {  // game entry point
    // do something ...
    // to use an api function :
    // api->function(args);
    return;
}
*/


#include <stdint.h>
#include <stddef.h>

#define PIN_BTN_A     14
#define PIN_BTN_B     13
#define PIN_BTN_UP    11
#define PIN_BTN_DOWN  9
#define PIN_BTN_LEFT  10
#define PIN_BTN_RIGHT 12

typedef struct 
{
    // DEBUG / HW
    int (*print_serial)(const char *, ...);  // printf through USB serial
    void (*backlight_state)(char);                 // control display backlight (0=off, 1=on)

    // GRAPHICS
    // > render buffer
    void(*flush_render_buffer)(void);
    // > pixel
    void (*set_px)(const uint16_t x_pos, const uint16_t y_pos, const uint16_t color);
    uint16_t (*get_px)(const uint16_t x_pos, const uint16_t y_pos);
    // > sprite rendering
    void (*put_sprite)(const uint16_t x_pos, const uint16_t y_pos, const uint16_t width, const uint16_t height, const uint16_t *sprite);

    // INPUTS
    char (*get_btn)(const uint32_t button);

    // MISC
    uint32_t (*get_rand_32)(void);
} os_api_t;



#endif // OS_API_H
