#ifndef GRAPH_H
#define GRAPH_H

#include "driver_screen.h"
#include "font.h"


#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF



uint16_t g_rgb24_to_rgb565(uint8_t r, uint8_t g, uint8_t b) ;
void g_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void g_draw_px(uint16_t x, uint16_t y, uint16_t color);
void g_draw_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* buffer);
void g_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg_color, uint16_t bg_color);
void g_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color);



uint16_t g_rgb24_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t r5 = (r >> 3) & 0x1F;   // 5 bits pour le rouge
    uint16_t g6 = (g >> 2) & 0x3F;   // 6 bits pour le vert
    uint16_t b5 = (b >> 3) & 0x1F;   // 5 bits pour le bleu

    // Combiner les composantes dans le format 16 bits : 5 bits rouge, 6 bits vert, 5 bits bleu
    return (r5 << 11) | (g6 << 5) | b5;
}

void g_fill_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {

    uint8_t data[4];
    uint16_t px_count;

    x1 += SCREEN_X_OFF;
    x2 += SCREEN_X_OFF;
    y1 += SCREEN_Y_OFF;
    y2 += SCREEN_Y_OFF;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 < 0) x2 = 0;
    if (y2 < 0) y2 = 0;

    if (x1 > SCREEN_WIDTH) x1 = SCREEN_WIDTH;
    if (x2 > SCREEN_WIDTH) x2 = SCREEN_WIDTH;
    if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;
    if (y2 > SCREEN_HEIGHT) y2 = SCREEN_HEIGHT;

    screen_write_command(CMD_CASET);
    data[0] = (x1 >> 8) & 0xFF;
    data[1] = x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] = x2 & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RASET);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] = y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] = y2 & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RAMWR);

    px_count = (x2 - x1 + 1) * (y2 - y1 + 1);
    
    data[0] = color >> 8;    // High byte of color
    data[1] = color & 0xFF;  // Low byte of color

    uint8_t pixel_data[512];  // Buffer temporaire pour accélérer l'envoi
    uint16_t index = 0;

    for (uint16_t i = 0; i < px_count; i++) {   
        pixel_data[index++] = data[0];
        pixel_data[index++] = data[1];

        if (index >= 512) {  // Envoyer par blocs de 256 pixels (512 bytes)
            screen_write_data(pixel_data, index);
            index = 0;
        }
    }
    if (index > 0) {  // Envoyer les restes
        screen_write_data(pixel_data, index);
    }
}

void g_draw_px(uint16_t x, uint16_t y, uint16_t color) {
    if ((x > SCREEN_WIDTH) || (y > SCREEN_HEIGHT + 1)) return;
    x = x + SCREEN_X_OFF;
    y = y + SCREEN_Y_OFF;

    uint8_t data[4];
    screen_write_command(CMD_CASET);
    data[0] = (x >> 8) & 0xFF;  
    data[1] = x & 0xFF;         
    data[2] = (x >> 8) & 0xFF;
    data[3] = x & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RASET);
    data[0] = (y >> 8) & 0xFF;  
    data[1] = y & 0xFF;         
    data[2] = (y >> 8) & 0xFF;
    data[3] = y & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RAMWR);
    data[0] = color >> 8;
    data[1] = color & 0xFF;
    screen_write_data(data, 2);
}


void g_draw_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* buffer) {
    uint8_t data[4];

    x += SCREEN_X_OFF;
    y += SCREEN_Y_OFF;

    screen_write_command(CMD_CASET);
    data[0] = (x >> 8) & 0xFF;
    data[1] = x & 0xFF;
    data[2] = ((x+w-1) >> 8) & 0xFF;
    data[3] = (x+w-1) & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RASET);
    data[0] = (y >> 8) & 0xFF;
    data[1] = y & 0xFF;
    data[2] = ((y+h-1) >> 8) & 0xFF;
    data[3] = (y+h-1) & 0xFF;
    screen_write_data(data, 4);

    screen_write_command(CMD_RAMWR);

    uint8_t pixel_data[512];  // Buffer temporaire pour accélérer l'envoi
    uint16_t index = 0;

    for (uint16_t i = 0; i < w*h; i++) {   
        pixel_data[index++] = buffer[i] >> 8;
        pixel_data[index++] = buffer[i] & 0xFF;

        if (index >= 512) {  // Envoyer par blocs de 256 pixels (512 bytes)
            screen_write_data(pixel_data, index);
            index = 0;
        }
    }
    if (index > 0) {  // Envoyer les restes
        screen_write_data(pixel_data, index);
    }
}

void g_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg_color, uint16_t bg_color) {
    const uint8_t *glyph = FONT_8x8[0];

    if (x < 0 || y < 0) return;
    if (x+FONT_WIDTH >= SCREEN_WIDTH || y+FONT_HEIGHT >= SCREEN_HEIGHT) return;


    if (c >= 'A' && c <= 'Z') glyph = FONT_8x8[c - 'A'+20];
    else if (c >= '0' && c <= '9') glyph = FONT_8x8[c - '0'+10];
    else { switch (c) {
        case '!': glyph = FONT_8x8[1]; break;
        case '.': glyph = FONT_8x8[2]; break;
        case ',': glyph = FONT_8x8[3]; break;
        case '?': glyph = FONT_8x8[4]; break;
        case '(': glyph = FONT_8x8[5]; break;
        case ')': glyph = FONT_8x8[6]; break;
        case '[': glyph = FONT_8x8[7]; break;
        case ']': glyph = FONT_8x8[8]; break;
        case '-': glyph = FONT_8x8[9]; break;
        case ':': glyph = FONT_8x8[46]; break;
        case '>': glyph = FONT_8x8[47]; break;
        case '<': glyph = FONT_8x8[48]; break;
    }}
        
    //const uint8_t *glyph = FONT_8x8[c - ' '];  // Récupère la police correspondante
    uint16_t buffer[FONT_WIDTH * FONT_HEIGHT];

    for (uint8_t i = 0; i < FONT_HEIGHT; i++) {
        for (uint8_t j = 0; j < FONT_WIDTH; j++) {
            buffer[i * FONT_WIDTH + j] = (glyph[i] & (1 << (7 - j))) ? fg_color : bg_color;
        }
    }

    g_draw_buffer(x, y, FONT_WIDTH, FONT_HEIGHT, buffer);
}

void g_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color) {
    while (*str) {
        g_draw_char(x, y, *str, fg_color, bg_color);
        x += FONT_WIDTH;
        str++;
    }
}


#endif