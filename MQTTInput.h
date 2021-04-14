#ifndef MQTT_INPUT_H
#define MQTT_INPUT_H

#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <PCF8575.h>

class MQTTInput {

    public:
        MQTTInput(uint8_t pinNumber, const char* inputTopic, PCF8575* pcf8575);

        void loop();

        void setDebounceDelay_mS(unsigned long debounceDelay_mS) {this->debounceDelay_mS = debounceDelay_mS;}
        //void setInputType(inputTypeEnum inputType) {this->inputType = inputType;}
        void setActiveMessage(const char* activeMessage) {this->activeMessage = activeMessage;}
        void setInactiveMessage(const char* inactiveMessage) {this->inactiveMessage = inactiveMessage;}

        uint8_t getPinNumber() {return this->pinNumber;}
        char* getPinString() {return this->pinString;}
        char* getPinID() {return this->pinID;}
        const char* getInputTopic() {return this->inputTopic;}

        void updateWebPage();

        void publishMQTTSensor();
        
    private:
        uint8_t pinNumber;
        char pinString[10];
        char pinID[10];
        PCF8575* pcf8575 = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to read the pins.
        const char* inputTopic;
        const char* activeMessage = "ACTIVE"; // If set will override the 'ACTIVE' message.
        const char* inactiveMessage = "INACTIVE"; // If set will override the 'INACTIVE' message.

        void readPin();

        unsigned long debounceDelay_mS = 50;
        int currentState;
        int lastState;
        unsigned long lastTime;

        void configurePin();
        int getPin();
};

#endif
