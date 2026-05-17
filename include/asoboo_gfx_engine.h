#pragma once
#include <LovyanGFX.hpp>
#include "asoboo_hw.h"


class AsobooGFXEngine {
public:
    static void init();
    static LGFX_Sprite* getCanvas();
    static void flip();
    static uint32_t getFps();

private:
    class AsobooDisplay: public lgfx::LGFX_Device {
        lgfx::Panel_ILI9341 _panel_instance;
        lgfx::Bus_Parallel8 _bus_instance;
    public:
        AsobooDisplay();
    };


    static AsobooDisplay _display;
    static LGFX_Sprite _buffers[2];
    static uint8_t _currentBuffer;
    
    // FPS
    static uint32_t _lastMillis;
    static uint32_t _frameCount;
    static uint32_t _fps;
};