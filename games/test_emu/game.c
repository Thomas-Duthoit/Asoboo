#include "os_api.h"


// Main game entry: called by _start with valid API pointer
__attribute__((used)) __attribute__((section(".text.game_main")))
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

    api->flush_render_buffer();
    return;
}