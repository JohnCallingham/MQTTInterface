#ifndef MQTT_RGB_LED_H
#define MQTT_RGB_LED_H

#include <RGB_LED_Controller.h>
#include <FastLED.h>
#include <Arduino.h>

class MQTT_RGB_LED {

    public:

        MQTT_RGB_LED(RGB_LED_Controller* rgb, uint8_t ledNumber, const char* ledTopic);

        const char* getRGB_LED_Topic() {return this->ledTopic;}

        void messageReceived(char* payload);

        void setOnColour(CRGB onColour) {this->onColour = onColour;}
        void setOffColour(CRGB offColour) {this->offColour = offColour;}

        void loop();

    private:

        RGB_LED_Controller* rgb;
        uint8_t ledNumber;
        const char* ledTopic;

        // Set the default on and off colours.
        CRGB onColour = CRGB::White;
        CRGB offColour = CRGB::Black;

};

#endif
