#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "asoboo_hw.h"

class LGFX_Asoboo : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance;
public:
    LGFX_Asoboo() {
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
        _panel_instance.config(panel_cfg);
        setPanel(&_panel_instance);
    }
};

LGFX_Asoboo display;

void setup() {
    Serial.begin(115200);

    display.init();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
}

void loop() {
    display.fillScreen(TFT_RED);
    display.print("RED");
    delay(1000);
    display.fillScreen(TFT_GREEN);
    display.print("GREEN");
    delay(1000);
}