#ifndef MQTT_INPUT_H
#define MQTT_INPUT_H

#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <PCF8575.h>

class MQTTInput {

    public:
        MQTTInput(uint8_t pinNumber, const char* sensorTopic, PCF8575* pcf8575);

        void loop();

        void setDebounceDelay_mS(unsigned long debounceDelay_mS) {this->debounceDelay_mS = debounceDelay_mS;}

        uint8_t getPinNumber() {return this->pinNumber;}
        char* getPinString() {return this->pinString;}
        char* getPinID() {return this->pinID;}
        const char* getSensorTopic() {return this->sensorTopic;}

        void updateWebPage();

        void publishMQTTSensor();
        
    private:
        uint8_t pinNumber;
        char pinString[10];
        char pinID[10];
        PCF8575* pcf8575 = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to read the pins.
        const char* sensorTopic;

        void readPin();

        unsigned long debounceDelay_mS = 50;
        int currentState;
        int lastState;
        unsigned long lastTime;

        void configurePin();
        int getPin();
};

#endif
