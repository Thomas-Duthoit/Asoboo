#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "asoboo_hw.h"

class AsobooDisplay : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance;
public:
    AsobooDisplay() {
        auto bus_cfg = _bus_instance.config();
        bus_cfg.port = 0;
        bus_cfg.freq_write = LCD_FREQ;
        bus_cfg.pin_wr = LCD_WR;
        bus_cfg.pin_rd = LCD_RD;
        bus_cfg.pin_rs = LCD_RS;
        bus_cfg.pin_d0 = LCD_D0; bus_cfg.pin_d1 = LCD_D1; bus_cfg.pin_d2 = LCD_D2; bus_cfg.pin_d3 = LCD_D3;
        bus_cfg.pin_d4 = LCD_D4; bus_cfg.pin_d5 = LCD_D5; bus_cfg.pin_d6 = LCD_D6; bus_cfg.pin_d7 = LCD_D7;
        _bus_instance.config(bus_cfg);
        _panel_instance.setBus(&_bus_instance);

        auto panel_cfg = _panel_instance.config();
        panel_cfg.pin_cs = LCD_CS;
        panel_cfg.pin_rst = LCD_RST;
        // panel_cfg.panel_width = SCR_WIDTH;
        // panel_cfg.panel_height = SCR_HEIGHT;
        _panel_instance.config(panel_cfg);
        setPanel(&_panel_instance);
    }
};

class AsobooGFX
{
private:
    AsobooDisplay display;
    LGFX_Sprite buffers[2];  // double buffering
    uint8_t currentBuffer = 0;

public:
    void init() {
        display.init();
        display.setRotation(1);

        for (int i = 0; i < 2; i++) {
            buffers[i].setPsram(true); // PSRAM allocation
            buffers[i].setColorDepth(16); // RGB565
            buffers[i].createSprite(SCR_WIDTH, SCR_HEIGHT);
        }
        Serial.println("[Asoboo] GFX Engine Ready");
    }


    LGFX_Sprite* getCanvas() {  // get the current drawing buffer
        return &buffers[currentBuffer];
    }


    void displayFrame() {  // push buffer to screen and switch to next one

        display.waitDisplay();
        
        buffers[currentBuffer].pushSprite(&display, 0, 0);

        currentBuffer = 1 - currentBuffer;  // switch to new buffer while the other one is beeing sent



    }
};

AsobooGFX GFX;


void setup() {
    Serial.begin(115200);

    Serial.println("[Asoboo] Kernel initialization...");

    GFX.init();

    Serial.println("[Asoboo] Kernel initialized...");

}

void loop() {
    static uint32_t lastMillis = 0;
    static uint32_t frameCount = 0;
    
    LGFX_Sprite* canvas = GFX.getCanvas();
    
    canvas->fillSprite(TFT_BLACK);
    
    static int x = 0;
    canvas->fillRoundRect(x, 100, 60, 60, 10, TFT_GOLD);
    canvas->drawRect(0, 0, 320, 240, TFT_WHITE);
    
    x = (x + 3) % 320;

    // FPS
    frameCount++;
    if (millis() - lastMillis >= 1000) {
        Serial.printf("FPS: %d\n", frameCount);
        frameCount = 0;
        lastMillis = millis();
    }

    GFX.displayFrame();
}