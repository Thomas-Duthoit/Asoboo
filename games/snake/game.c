#include "os_api.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SQUARE_SIZE 16




void fill_square(const os_api_t *api, char grid[][SCREEN_WIDTH/SQUARE_SIZE], const uint16_t x, const uint16_t y, const uint16_t color) {
    api->print_serial("TEST LOUIS");
    for (uint16_t i=0; i<SQUARE_SIZE; i++) {
        for (uint16_t j=0; j<SQUARE_SIZE; j++) {
            api->set_px(x+i, y+j, color);
        }
    }
}


// void render_grid(const os_api_t *api, char grid[][SCREEN_WIDTH/SQUARE_SIZE]) {
//     for (int y = 0; y<SCREEN_HEIGHT/SQUARE_SIZE; y++) {
//         for (int x = 0; x<SCREEN_WIDTH/SQUARE_SIZE; x++) {
//             api->print_serial("FLAG");
//             if (grid[y][x] == 0)
//                 continue;
//             if (grid[y][x] == 1)
//                 fill_square(api, grid, x, y, 0x0FF0);
//             if (grid[y][x] == 2)
//                 fill_square(api, grid, x, y, 0xF200);
//         }
//     }
// }






__attribute__((used)) __attribute__((section(".text.game_main")))
void game_main(os_api_t* api) {
    char grid[SCREEN_HEIGHT/SQUARE_SIZE][SCREEN_WIDTH/SQUARE_SIZE];
    api->backlight_state(1);


    for (int y = 0; y<SCREEN_HEIGHT/SQUARE_SIZE; y++) {
        for (int x = 0; x<SCREEN_WIDTH/SQUARE_SIZE; x++) {
            api->print_serial("x=%d, y=%d\n", x, y);
            grid[y][x] = 0;
        }
    }

    api->print_serial("INIT TERMINEE\n");


    grid[3][3] = 1;
    grid[5][5] = 2;

    // render_grid(api, grid);

    for (uint16_t y = 0; y<SCREEN_HEIGHT/SQUARE_SIZE; y++) {
        for (uint16_t x = 0; x<SCREEN_WIDTH/SQUARE_SIZE; x++) {
            // api->print_serial("FLAG");
            if (grid[y][x] == 0)
                continue;
            if (grid[y][x] == 1)
                fill_square(api, grid, x, y, 0x0FF0);
            if (grid[y][x] == 2)
                fill_square(api, grid, x, y, 0xF200);
        }
    }

    api->print_serial("RENDER TERMINEE\n");

    // mainloop
    while (1) {
        // exit condition
        if (api->get_btn(BTN_HOME)) {
            return;
        }



        api->flush_render_buffer(); // flush buffer
    }
}