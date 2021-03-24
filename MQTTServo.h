#ifndef MQTT_SERVO_H
#define MQTT_SERVO_H

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_PWMServoDriver.h>

class MQTTServo {
    public:
        // MQTTServo(uint8_t pinNumber, const char* turnoutTopic, Adafruit_PWMServoDriver* pwm);
        MQTTServo(uint8_t pinNumber, const char* turnoutTopic);
        MQTTServo(uint8_t pinNumber, const char* turnoutTopic, Adafruit_PWMServoDriver* pwm)
            : MQTTServo(pinNumber, turnoutTopic) {this->pwm = pwm;}

        enum receivedMessageEnum {
            messageThrown,
            messageClosed,
            reachedThrown,
            reachedClosed
        };

        void setMQTTBroker(const char* mqttBroker) {this->mqttBroker = mqttBroker;}
        void setMQTTPort(uint16_t mqttPort) {this->mqttPort = mqttPort;}
        void setAngleClosed(int angleClosed) {this->angleClosed = angleClosed;}
        void setAngleThrown(int angleThrown) {this->angleThrown = angleThrown;}
        void setTimeFromClosedToThrown_mS(unsigned long timeFromClosedToThrown_mS) {this->timeFromClosedToThrown_mS = timeFromClosedToThrown_mS;}
        void setTimeFromThrownToClosed_mS(unsigned long timeFromThrownToClosed_mS) {this->timeFromThrownToClosed_mS = timeFromThrownToClosed_mS;}
        void setThrownSensorTopic(const char* thrownSensorTopic) {this->thrownSensorTopic = thrownSensorTopic;}
        void setClosedSensorTopic(const char* closedSensorTopic) {this->closedSensorTopic = closedSensorTopic;}

        uint8_t getPinNumber() {return this->pinNumber;}
        const char* getTurnoutTopic() {return this->turnoutTopic;}

        void loop();

        void messageReceived(receivedMessageEnum message);
        void updateWebPage();

    private:
        enum stateEnum {
            stateUndefined, //0
            stateThrown, //1
            stateMoving_Towards_Closed, //2
            stateClosed, //3
            stateMoving_Towards_Thrown  //4
        };

        stateEnum currentState = stateUndefined;

        uint8_t pinNumber;
        Adafruit_PWMServoDriver* pwm = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to drive the pins.
        const char* turnoutTopic;
        WiFiClient wifiClient;

        const char* mqttBroker = "raspberrypi";
        uint16_t mqttPort = 1883;
        int angleThrown = 60;
        int angleClosed = 50;
        unsigned long timeFromThrownToClosed_mS = 1000;
        unsigned long timeFromClosedToThrown_mS = 1000;
        const char* thrownSensorTopic = "";
        const char* closedSensorTopic = "";

        int currentServoAngle;
        unsigned long movePeriodToClosed_mS;
        unsigned long movePeriodToThrown_mS;
        unsigned long lastTimeServoMoved;

        void calculatePeriods(); 
        void handleStateTransition(stateEnum newState, const char* thrownSensorMessage, const char* closedSensorMessage);
        const char* stateString(stateEnum state);
        void adjustServoPosition();
        void adjustMovingTowardsClosed();
        void adjustMovingTowardsThrown();
        void publishMQTTSensor(const char* topic, const char* payload);

        bool initialised = false;

        void configurePin();
        void updatePin(uint8_t newValue);        
};

#endif
