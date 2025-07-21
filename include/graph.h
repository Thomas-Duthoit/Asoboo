#ifndef GRAPH_H
#define GRAPH_H

#include "driver_screen.h"

#define get_buff_idx(x, y)  SCREEN_WIDTH * y + x  // macro function to convert x;y pos to buffer index
#define endianswap16(x) (((x) >> 8) | ((x) << 8))


uint16_t render_buffer[SCREEN_HEIGHT*SCREEN_WIDTH];  // primary render buffer


void graph_flush_render_buffer(void) {
    uint8_t data[4];

    screen_write_command(CMD_CASET);
    data[0] = 0 >> 8;   
    data[1] = 0 & 0xFF;   
    data[2] = (SCREEN_WIDTH - 1) >> 8;
    data[3] = (SCREEN_WIDTH - 1) & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RASET);
    data[0] = 0 >> 8;   
    data[1] = 0 & 0xFF;   
    data[2] = (SCREEN_HEIGHT - 1) >> 8;
    data[3] = (SCREEN_HEIGHT - 1) & 0xFF;    
    screen_write_data(data, 4);

    screen_write_command(CMD_RAMWR);
    screen_write_data((uint8_t*)render_buffer, 2*SCREEN_HEIGHT*SCREEN_WIDTH);
}


void graph_set_px (const uint16_t x_pos, const uint16_t y_pos, const uint16_t color) {
    if ((x_pos >= 0 && x_pos < SCREEN_WIDTH) && (y_pos >= 0 && y_pos < SCREEN_HEIGHT)) 
        render_buffer[get_buff_idx(x_pos, y_pos)] = endianswap16(color);
}


uint16_t graph_get_px (const uint16_t x_pos, const uint16_t y_pos) {
    if ((x_pos >= 0 && x_pos < SCREEN_WIDTH) && (y_pos >= 0 && y_pos < SCREEN_HEIGHT))
        return endianswap16(render_buffer[get_buff_idx(x_pos, y_pos)]);
    else
        return 0;
}


void graph_put_sprite (const uint16_t x_pos, const uint16_t y_pos, const uint16_t width, const uint16_t height, const uint16_t *sprite) {
    for (uint16_t y = 0; y < height; y++) {
        uint16_t screen_y = y_pos + y;
        if (screen_y >= SCREEN_HEIGHT) continue; // Avoid vertical overflow

        for (uint16_t x = 0; x < width; x++) {
            uint16_t screen_x = x_pos + x;
            if (screen_x >= SCREEN_WIDTH) continue; // Avoid horizontal overflow

            render_buffer[get_buff_idx(screen_x, screen_y)] = endianswap16(sprite[y * width + x]);
        }
    }
}

void graph_fill_rect (const uint16_t x_pos, const uint16_t y_pos, const uint16_t width, const uint16_t height, const uint16_t color) {
    for (uint16_t y = 0; y < height; y++) {
        uint16_t screen_y = y_pos + y;
        if (screen_y >= SCREEN_HEIGHT) continue; // Avoid vertical overflow

        for (uint16_t x = 0; x < width; x++) {
            uint16_t screen_x = x_pos + x;
            if (screen_x >= SCREEN_WIDTH) continue; // Avoid horizontal overflow

            render_buffer[get_buff_idx(screen_x, screen_y)] = endianswap16(color);
        }
    }
}



#endif //  GRAPH_H