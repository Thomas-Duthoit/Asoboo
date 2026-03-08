#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../include/pinhout.h"


#define NUM_LEDS 1

Adafruit_NeoPixel led(NUM_LEDS, PIN_RGB_LED, NEO_GRB + NEO_KHZ800);

void setup() {
    led.begin();
    led.setBrightness(255);
    led.show();
}

void loop() {
    led.setPixelColor(0, led.Color(0, 0, 255)); // Blue
    led.show();
    delay(500);

    led.setPixelColor(0, led.Color(255, 255, 255)); // White
    led.show();
    delay(500);

    led.setPixelColor(0, led.Color(255, 0, 0)); // Red
    led.show();
    delay(500);

    led.setPixelColor(0, led.Color(0, 0, 0)); // OFF
    led.show();
    delay(500);
}
