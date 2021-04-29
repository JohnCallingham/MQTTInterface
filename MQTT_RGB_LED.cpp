#include <MQTT_RGB_LED.h>

MQTT_RGB_LED::MQTT_RGB_LED(RGB_LED_Controller* rgb, uint8_t ledNumber, const char* ledTopic) {
    this->rgb = rgb;
    this->ledNumber = ledNumber;
    this->ledTopic = ledTopic;
}

void MQTT_RGB_LED::loop() {
    // If this LED has a non zero onTime_mS or offTime_mS then this sets the blinking flag.
    // When the blinking flag is set an ON message causes the LED to alternatively display the onColour for the onTime_mS
    //  and then the offColour for the offTime_mS.
    // When the OFF message is received the offColour will be displayed permanently.

    if (blinking) {
        if (this->rgb->leds[this->ledNumber] == this->onColour) { // Assumes that the on colour is different to the off colour!!
            // The LED is on.
            if ((timeToTurnLEDOff != 0) && (millis() >= timeToTurnLEDOff)) {
                turnLEDOff();
                timeToTurnLEDOn = millis() + offTime_mS;
            }
        } else {
            // The LED is off.
            if ((timeToTurnLEDOn != 0) && (millis() >= timeToTurnLEDOn)) {
                turnLEDOn();
                timeToTurnLEDOff = millis() + onTime_mS;
            }
        }
    }
}

void MQTT_RGB_LED::messageReceived(char* payload) {
    if (strcmp(payload, "ON") == 0) {
        // Check whether this LED is set to blink.
        if ((this->onTime_mS > 0) || (this->offTime_mS > 0)) {
            blinking = true;
        } else {
            blinking = false;
            Serial.printf("LED number %i turned on 0x%02Xx%02Xx%02X\n", this->ledNumber, this->onColour.raw[0], this->onColour.raw[1], this->onColour.raw[2]);
            // this->rgb->leds[this->ledNumber] = this->onColour;
            // FastLED.show();
            turnLEDOn();
        }
    }

    if (strcmp(payload, "OFF") == 0) {
        blinking = false;
        Serial.printf("LED number %i turned off 0x%02Xx%02Xx%02X\n", this->ledNumber, this->onColour.raw[0], this->onColour.raw[1], this->onColour.raw[2]);
        // this->rgb->leds[this->ledNumber] = this->offColour;
        // FastLED.show();
        turnLEDOff();
    }
}

void MQTT_RGB_LED::turnLEDOn() {
    this->rgb->leds[this->ledNumber] = this->onColour;
    FastLED.show();
}

void MQTT_RGB_LED::turnLEDOff() {
    this->rgb->leds[this->ledNumber] = this->offColour;
    FastLED.show();
}
