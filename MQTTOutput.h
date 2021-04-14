#ifndef MQTT_OUTPUT_H
#define MQTT_OUTPUT_H
#include <PCF8575.h>
#include <Arduino.h>

class MQTTOutput {
    public:

        MQTTOutput(uint8_t pinNumber, const char* outputTopic, PCF8575* pcf8575);

        uint8_t getPinNumber() {return this->pinNumber;}
        char* getPinString() {return this->pinString;}
        char* getPinID() {return this->pinID;}
        const char* getOutputTopic() {return this->outputTopic;}

        void updateWebPage();

        void messageReceived(char* payload);

    private:

        uint8_t pinNumber;
        char pinString[10];
        char pinID[10];
        PCF8575* pcf8575 = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to drive the pins.
        const char* outputTopic;

        char currentState[20];

        void configurePin();
        void updatePin(uint8_t newValue);
};

#endif
