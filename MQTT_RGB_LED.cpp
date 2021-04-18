#include <MQTT_RGB_LED.h>

MQTT_RGB_LED::MQTT_RGB_LED(RGB_LED_Controller* rgb, uint8_t ledNumber, const char* ledTopic) {
    this->rgb = rgb;
    this->ledNumber = ledNumber;
    this->ledTopic = ledTopic;
}

void MQTT_RGB_LED::loop() {

// is this required????
    
}

void MQTT_RGB_LED::messageReceived(char* payload) {
    if (strcmp(payload, "ON") == 0) {
        Serial.printf("LED number %i turned on 0x%08X\n", this->ledNumber, this->onColour);
        this->rgb->leds[this->ledNumber] = this->onColour;
        FastLED.show();
    }
    if (strcmp(payload, "OFF") == 0) {
        Serial.printf("LED number %i turned off 0x%08X\n", this->ledNumber, this->offColour);
        this->rgb->leds[this->ledNumber] = this->offColour;
        FastLED.show();
    }
}
