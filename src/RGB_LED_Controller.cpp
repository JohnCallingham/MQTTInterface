#include <RGB_LED_Controller.h>

RGB_LED_Controller::RGB_LED_Controller() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN> (this->leds, NUM_LEDS);
}

RGB_LED_Controller::RGB_LED_Controller(uint8_t brightness) {
    this->brightness = brightness;
    FastLED.addLeds<NEOPIXEL, DATA_PIN> (this->leds, NUM_LEDS);
}
