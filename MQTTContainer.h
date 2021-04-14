#ifndef MQTT_CONTAINER_H
#define MQTT_CONTAINER_H

#include <MQTTServo.h>
#include <MQTTOutput.h>
#include <MQTTInput.h>
#include <ESP8266WebServer.h>
#include <list>

class MQTTContainer {

    public:
        MQTTContainer();
        
        MQTTServo* addServo(uint8_t pinNumber, const char* servoTopic);
        MQTTServo* addServo(uint8_t pinNumber, const char* servoTopic, Adafruit_PWMServoDriver* pwm);

        MQTTOutput* addOutput(uint8_t pinNumber, const char* outputTopic);
        MQTTOutput* addOutput(uint8_t pinNumber, const char* outputTopic, PCF8575* pcf8575);

        MQTTInput* addInput(uint8_t pinNumber, const char* inputTopic);
        MQTTInput* addInput(uint8_t pinNumber, const char* inputTopic, PCF8575* pcf8575);


        void loop();

        void setBroker(const char* mqttBroker) {this->mqttBroker = mqttBroker;}
        void setPort(uint16_t mqttPort) {this->mqttPort = mqttPort;}
        void setStartupTopic(const char* startupTopic) {this->startupTopic = startupTopic;}

    private:
        std::list<MQTTServo*> servoList;
        std::list<MQTTOutput*> outputList;
        std::list<MQTTInput*> inputList;

        const char* mqttBroker = "raspberrypi";
        uint16_t mqttPort = 1883;
        const char* startupTopic = "events";

        String indexWebPage = "";
        String servosWebPage = "";
        String outputsWebPage = "";
        String inputsWebPage = "";

        void handleNewWebSocketClient();
        void connectToMQTT();
        void buildIndexWebPage();
        void buildServosWebPage();
        String getRepeatingText();
        void buildOutputsWebPage();
        void buildInputsWebPage();
        void publishStartupMessage();
        void callback(char* topic, byte* payload, unsigned int length);
        String replaceAll(String s);
        String getServosJSON();
        String getBasicRelaysJSON();
        String getAdvancedRelaysJSON();
        String getSensorsJSON();

        void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
        // MQTTServo* determineServo(uint8_t *payload);
        // MQTTServo* determineServo(uint8_t pinNumber);
        MQTTServo* determineServo(char* pinString);

        ESP8266WebServer server;

        int connectedClients = 0;
        bool initialised = false;
};

#endif
