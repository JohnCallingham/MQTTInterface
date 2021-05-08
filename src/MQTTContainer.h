#ifndef MQTT_CONTAINER_H
#define MQTT_CONTAINER_H

#include <MQTTServo.h>
#include <MQTTOutput.h>
#include <MQTTInput.h>
#include <MQTT_RGB_LED.h>
#include <RGB_LED_Controller.h>
//#include <FastLED.h>
#include <ESP8266WebServer.h>
#include <list>
#include <Blink.h>

class MQTTContainer {

    public:
        MQTTContainer();
        
        MQTTServo* addServo(uint8_t pinNumber, const char* servoTopic, Adafruit_PWMServoDriver* pwm);

        MQTTOutput* addOutput(uint8_t pinNumber, const char* outputTopic);
        MQTTOutput* addOutput(uint8_t pinNumber, const char* outputTopic, Adafruit_MCP23017* mcp);

        MQTTInput* addInput(uint8_t pinNumber, const char* inputTopic);
        MQTTInput* addInput(uint8_t pinNumber, const char* inputTopic, Adafruit_MCP23017* mcp);

        MQTT_RGB_LED* addRGB_LED(uint8_t ledNumber, const char* ledTopic, RGB_LED_Controller* rgb);

        Blink* addBlink(unsigned long onTime_mS, unsigned long offTime_mS);

        void loop();

        void setBroker(const char* mqttBroker) {this->mqttBroker = mqttBroker;}
        void setPort(uint16_t mqttPort) {this->mqttPort = mqttPort;}
        void setLogTopic(const char* logTopic) {this->logTopic = logTopic;}

        void sendLogMessage(const char* s, ...);

    private:
        std::list<MQTTServo*> servoList;
        std::list<MQTTOutput*> outputList;
        std::list<MQTTInput*> inputList;
        std::list<MQTT_RGB_LED*> rgbLEDList;
        std::list<Blink*> blinkList;

        const char* mqttBroker = "raspberrypi";
        uint16_t mqttPort = 1883;
        //const char* startupTopic = "events";
        const char* logTopic = "events";

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
        String getOutputsJSON();
        String getInputsJSON();

        void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
        // MQTTServo* determineServo(uint8_t *payload);
        // MQTTServo* determineServo(uint8_t pinNumber);
        MQTTServo* determineServo(char* pinString);

        bool servoPinAlreadyInUse();

        ESP8266WebServer server;

        int connectedClients = 0;
        bool initialised = false;
};

#endif
