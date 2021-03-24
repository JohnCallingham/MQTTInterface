#ifndef MQTT_CONTAINER_H
#define MQTT_CONTAINER_H

#include <MQTTServo.h>
#include <MQTTRelay.h>
#include <MQTTSensor.h>
#include <ESP8266WebServer.h>
#include <list>

class MQTTContainer {

    public:
        MQTTContainer();
        
        MQTTServo* addServo(uint8_t pinNumber, const char* servoTopic);
        MQTTServo* addServo(uint8_t pinNumber, const char* servoTopic, Adafruit_PWMServoDriver* pwm);

        MQTTRelay* addRelay(uint8_t pinNumber, const char* relayTopic);
        MQTTRelay* addRelay(uint8_t pinNumber, const char* relayTopic, PCF8575* pcf8575);
        MQTTRelay* addRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic);
        MQTTRelay* addRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic, PCF8575* pcf8575);

        MQTTSensor* addSensor(uint8_t pinNumber, const char* sensorTopic);
        MQTTSensor* addSensor(uint8_t pinNumber, const char* sensorTopic, PCF8575* pcf8575);

        void loop();

        void setBroker(const char* mqttBroker) {this->mqttBroker = mqttBroker;}
        void setPort(uint16_t mqttPort) {this->mqttPort = mqttPort;}
        void setStartupTopic(const char* startupTopic) {this->startupTopic = startupTopic;}

    private:
        std::list<MQTTServo*> servoList;
        std::list<MQTTRelay*> relayBasicList;
        std::list<MQTTRelay*> relayAdvancedList;
        std::list<MQTTSensor*> sensorList;

        const char* mqttBroker = "raspberrypi";
        uint16_t mqttPort = 1883;
        const char* startupTopic = "events";

        String indexWebPage = "";
        String servosWebPage = "";
        String basicRelaysWebPage = "";
        String advancedRelaysWebPage = "";
        String sensorsWebPage = "";

        void handleNewWebSocketClient();
        void connectToMQTT();
        void buildIndexWebPage();
        void buildServosWebPage();
        void buildBasicRelaysWebPage();
        void buildAdvancedRelaysWebPage();
        void buildSensorsWebPage();
        void publishStartupMessage();
        void callback(char* topic, byte* payload, unsigned int length);
        String replaceAll(String s);
        String getServosJSON();
        String getBasicRelaysJSON();
        String getAdvancedRelaysJSON();
        String getSensorsJSON();

        ESP8266WebServer server;

        int connectedClients = 0;
        bool initialised = false;
};

#endif
