#ifndef RGB_LED_CONTROLLER_H
#define RGB_LED_CONTROLLER_H

#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 1
#define DATA_PIN 13

class RGB_LED_Controller {

    public:

        RGB_LED_Controller();
        RGB_LED_Controller(uint8_t brightness);

        // There doesn't appear to be any easy way of dynamically setting the number of LEDs.
        // The answer appears to be to set a large number and ignore any extra ones.
        // https://forum.arduino.cc/t/fastled-change-led-number-at-run-time/674819/23
        CRGB leds[NUM_LEDS]; // The array of LEDs.

        uint8_t getBrightness() {return this->brightness;}

    private:

        uint8_t brightness = 50;

};

#endif
