#include <Arduino.h>
#include "asoboo_hw.h"
#include "asoboo_gfx_engine.h"

void setup() {
    Serial.begin(115200);

    Serial.println("[Asoboo] Kernel initialization...");

    
    AsobooGFXEngine::init();

    Serial.println("[Asoboo] Kernel initialized...");

}

void loop() {
    static uint32_t lastMillis = 0;
    static uint32_t frameCount = 0;
    
    LGFX_Sprite* canvas = AsobooGFXEngine::getCanvas();
    
    canvas->fillSprite(TFT_BLACK);
    
    static int x = 0;
    canvas->fillRoundRect(x, 100, 60, 60, 10, TFT_GOLD);
    canvas->drawRect(0, 0, 320, 240, TFT_WHITE);
    
    x = (x + 3) % 320;

    canvas->setCursor(5, 5);
    canvas->printf("FPS : %d", AsobooGFXEngine::getFps());

    AsobooGFXEngine::flip();
}