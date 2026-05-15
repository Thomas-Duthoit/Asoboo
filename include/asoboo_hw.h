#pragma once

// SCREEN (ILI9341)
#define LCD_D0   1
#define LCD_D1   2
#define LCD_D2   3
#define LCD_D3   4
#define LCD_D4   5
#define LCD_D5   6
#define LCD_D6   7
#define LCD_D7   8

#define LCD_RS   9   // Register Select (Data/Cmd)
#define LCD_WR   10  // Write
#define LCD_RD   11  // Read
#define LCD_CS   12  // Chip Select
#define LCD_RST  13  // Reset

#define LCD_FREQ 20000000 // 20 MHz for parallel bus
#define SCR_WIDTH  320
#define SCR_HEIGHT 240

// I2S AUDIO
// #define I2S_BCLK 14
// #define I2S_LRCK 15
// #define I2S_DOUT 16

// INPUTS
// #define SR_LATCH 40
// #define SR_CLK   41
// #define SR_DATA  42

// LED
#define EMB_LED 48
