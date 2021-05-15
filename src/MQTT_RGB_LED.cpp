#include <MQTT_RGB_LED.h>

MQTT_RGB_LED::MQTT_RGB_LED(RGB_LED_Controller* rgb, uint8_t ledNumber, const char* ledTopic) {
    this->rgb = rgb;
    this->ledNumber = ledNumber;
    this->ledTopic = ledTopic;
}

void MQTT_RGB_LED::loop() {
    if (ledMessageOn && blinking) {
        if (this->rgb->leds[this->ledNumber] == this->onColour) { // Assumes that the on colour is different to the off colour!!
            // The LED is on.
            if (!blink->ledStateOn) turnLEDOff();
        } else {
            // The LED is off.
            if (blink->ledStateOn) turnLEDOn();
        }
    }
}

void MQTT_RGB_LED::messageReceived(char* payload) {
    if ((strcmp(payload, "ON") == 0) || (strcmp(payload, "ACTIVE") == 0)) {
        // Turn LED on or start blinking.
        ledMessageOn = true;
        if (!blinking) turnLEDOn();
    }

    if ((strcmp(payload, "OFF") == 0) || (strcmp(payload, "INACTIVE") == 0)) {
        // Turn LED off or stop blinking.
        ledMessageOn = false;
        turnLEDOff();
    }
}

void MQTT_RGB_LED::turnLEDOn() {
    this->rgb->leds[this->ledNumber] = this->onColour;
    FastLED.setBrightness(this->rgb->getBrightness());
    FastLED.show();
    //Serial.printf("LED number %i turned on 0x%02Xx%02Xx%02X\n", this->ledNumber, this->onColour.raw[0], this->onColour.raw[1], this->onColour.raw[2]);
}

void MQTT_RGB_LED::turnLEDOff() {
    this->rgb->leds[this->ledNumber] = this->offColour;
    FastLED.setBrightness(this->rgb->getBrightness());
    FastLED.show();
    //Serial.printf("LED number %i turned off 0x%02Xx%02Xx%02X\n", this->ledNumber, this->onColour.raw[0], this->onColour.raw[1], this->onColour.raw[2]);
}
