#include "os_api.h"


uint16_t rgb888_to_bgr565_no_div(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t r_5 = r >> 3;
    uint16_t g_6 = g >> 2;
    uint16_t b_5 = b >> 3;
    return (r_5 << 11) | (g_6 << 5) | b_5;
}


// Main game entry: called by _start with valid API pointer
__attribute__((used)) __attribute__((section(".text.game_main")))
void game_main(os_api_t* api) {

    uint8_t r, g, b;

    for (int i=0; i<128; i++) {
        r=i*2;
        for (int j=0; j<128; j++) {
            b=j*2;
            api->set_px(i+16, j, rgb888_to_bgr565_no_div(r, 0, b));
        }
    }

    api->flush_render_buffer();
    return;
}