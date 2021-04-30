#include <RGB_LED_Controller.h>

RGB_LED_Controller::RGB_LED_Controller() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN> (this->leds, NUM_LEDS);
}
