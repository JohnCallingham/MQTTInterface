#ifndef MQTT_OUTPUT_H
#define MQTT_OUTPUT_H
//#include <PCF8575.h>
#include <Adafruit_MCP23017.h>
#include <Arduino.h>

class MQTTOutput {
    public:

        MQTTOutput(uint8_t pinNumber, const char* outputTopic, Adafruit_MCP23017* mcp);

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
        Adafruit_MCP23017* mcp = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to drive the pins.
        const char* outputTopic;

        char currentState[20];

        void configurePin();
        void updatePin(uint8_t newValue);
};

#endif
