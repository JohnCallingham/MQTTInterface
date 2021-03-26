#ifndef MQTT_RELAY_H
#define MQTT_RELAY_H
#include <PCF8575.h>
#include <Arduino.h>

class MQTTRelay {
    public:

        //MQTTRelay(uint8_t pinNumber, const char* relayTopic);
        //MQTTRelay(uint8_t pinNumber, const char* relayTopic, PCF8575* pcf8575) 
            //: MQTTRelay(pinNumber, relayTopic) {this->pcf8575 = pcf8575;}
        MQTTRelay(uint8_t pinNumber, const char* relayTopic, PCF8575* pcf8575);
        //MQTTRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic);
        //MQTTRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic, PCF8575* pcf8575)
            //: MQTTRelay(pinNumber, relayOperateTopic, relayReleaseTopic) {this->pcf8575 = pcf8575;}
        MQTTRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic, PCF8575* pcf8575);

        uint8_t getPinNumber() {return this->pinNumber;}
        const char* getRelayTopic() {return this->relayTopic;}
        const char* getRelayOperateTopic() {return this->relayOperateTopic;}
        const char* getRelayReleaseTopic() {return this->relayReleaseTopic;}

        void updateWebPage();

        void receivedRelayTopic(char* payload);
        void receivedRelayOperateTopic(char* payload);
        void receivedRelayReleaseTopic(char* payload);

    private:

        uint8_t pinNumber;
        PCF8575* pcf8575 = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to drive the pins.
        const char* relayTopic;
        const char* relayOperateTopic;
        const char* relayReleaseTopic;

        char currentState[20];

        void configurePin();
        void updatePin(uint8_t newValue);
};

#endif
