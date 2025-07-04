
// Minimal Asoboo game binary entry point
#include "os_api.h"


// Forward declaration of game_main (called from _start)
__attribute__((used)) __attribute__((section(".text.game_main"))) __attribute__((noreturn))
void game_main(os_api_t* api);


// Minimal startup: pass API pointer to game_main, then halt
__attribute__((section(".text._start")))
void _start(void) {
    os_api_t* api = (os_api_t*)0x20020000; // API table address fixed by emulator
    game_main(api); // never returns
    while (1) { __asm__ volatile ("nop"); } // safety loop
}

// Main game entry: called by _start with valid API pointer
__attribute__((used)) __attribute__((section(".text.game_main"))) __attribute__((noreturn))
void game_main(os_api_t* api) {

    // Draw a single white diagonal line using Bresenham's algorithm
    int x0 = 10, y0 = 10, x1 = 100, y1 = 100;
    int dx = x1 - x0, dy = y1 - y0;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    dx = sx * dx;
    dy = sy * dy;
    int err = dx - dy;
    while (1) {
        api->set_px(x0, y0, 0xFFFF); // set pixel to white
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}