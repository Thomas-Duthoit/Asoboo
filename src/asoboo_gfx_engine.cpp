#include "asoboo_gfx_engine.h"


// static
AsobooGFXEngine::AsobooDisplay AsobooGFXEngine::_display;
LGFX_Sprite AsobooGFXEngine::_buffers[2];
uint8_t AsobooGFXEngine::_currentBuffer = 0;
uint32_t AsobooGFXEngine::_lastMillis = 0;
uint32_t AsobooGFXEngine::_frameCount = 0;
uint32_t AsobooGFXEngine::_fps = 0;

// hardware display constructor
AsobooGFXEngine::AsobooDisplay::AsobooDisplay() {
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

void AsobooGFXEngine::init() {
    _display.init();
    _display.setRotation(1);

    for (int i = 0; i < 2; i++) {
        _buffers[i].setPsram(true);
        _buffers[i].setColorDepth(16);
        _buffers[i].createSprite(SCR_WIDTH, SCR_HEIGHT);
        _buffers[i].setTextSize(2);
    }

    getCanvas()->fillScreen(TFT_BLACK);
    flip();
}

LGFX_Sprite* AsobooGFXEngine::getCanvas() {
    return &_buffers[_currentBuffer];
}

void AsobooGFXEngine::flip() {
    _display.waitDisplay();
    
    _buffers[_currentBuffer].pushSprite(&_display, 0, 0);
    
    _currentBuffer = 1 - _currentBuffer;

    // FPS compute
    _frameCount++;
    if (millis() - _lastMillis >= 1000) {
        _fps = _frameCount;
        _frameCount = 0;
        _lastMillis = millis();
    }
}

uint32_t AsobooGFXEngine::getFps() {
    return _fps;
}