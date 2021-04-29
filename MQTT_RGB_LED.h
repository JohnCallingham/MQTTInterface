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
        void setOnTime(unsigned long onTime_mS) {this->onTime_mS = onTime_mS; this->timeToTurnLEDOn = 1;}
        void setOffTime(unsigned long offTime_mS) {this->offTime_mS = offTime_mS; this->timeToTurnLEDOff = 1;}

        void loop();

    private:

        RGB_LED_Controller* rgb;
        uint8_t ledNumber;
        const char* ledTopic;
        bool blinking = false;
        unsigned long timeToTurnLEDOff = 0;
        unsigned long timeToTurnLEDOn = 0;

        // Set the default on and off colours.
        CRGB onColour = CRGB::White;
        CRGB offColour = CRGB::Black;

        // Set the default on and off times for no blinking.
        unsigned long onTime_mS = 0;
        unsigned long offTime_mS = 0;

        void turnLEDOn();
        void turnLEDOff();

};

#endif
