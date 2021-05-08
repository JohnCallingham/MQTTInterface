#ifndef MQTT_RGB_LED_H
#define MQTT_RGB_LED_H

#include <RGB_LED_Controller.h>
#include <FastLED.h>
#include <Arduino.h>
#include <Blink.h>

class MQTT_RGB_LED {

    public:

        MQTT_RGB_LED(RGB_LED_Controller* rgb, uint8_t ledNumber, const char* ledTopic);

        const char* getRGB_LED_Topic() {return this->ledTopic;}
        uint8_t getLedNumber() {return this->ledNumber;}

        void messageReceived(char* payload);

        void setOnColour(CRGB onColour) {this->onColour = onColour;}
        void setOffColour(CRGB offColour) {this->offColour = offColour;}

        void setBlink(Blink* blink) {this->blink = blink; this->blinking = true;}

        void loop();

    private:

        bool ledMessageOn = false; // Set by the MQTT message received.

        RGB_LED_Controller* rgb;
        uint8_t ledNumber;
        const char* ledTopic;
        bool blinking = false;

        // Set the default on and off colours.
        CRGB onColour = CRGB::White;
        CRGB offColour = CRGB::Black;

        Blink* blink = NULL;

        void turnLEDOn();
        void turnLEDOff();

};

#endif
