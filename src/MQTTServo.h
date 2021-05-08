#ifndef MQTT_SERVO_H
#define MQTT_SERVO_H

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_PWMServoDriver.h>

class MQTTServo {
    public:
        MQTTServo(uint8_t pinNumber, const char* turnoutTopic, Adafruit_PWMServoDriver* pwm);

        enum receivedMessageEnum {
            messageThrown,
            messageClosed,
            reachedThrown,
            reachedClosed,
            reachedMidPoint
        };

        void setAngleClosed(int angleClosed);
        void setAngleThrown(int angleThrown);
        void setTimeFromClosedToThrown_mS(unsigned long timeFromClosedToThrown_mS) {this->timeFromClosedToThrown_mS = timeFromClosedToThrown_mS;}
        void setTimeFromThrownToClosed_mS(unsigned long timeFromThrownToClosed_mS) {this->timeFromThrownToClosed_mS = timeFromThrownToClosed_mS;}
        void setThrownTopic(const char* thrownTopic) {this->thrownTopic = thrownTopic;}
        void setClosedTopic(const char* closedTopic) {this->closedTopic = closedTopic;}
        void setMidPointTopic(const char* midPointTopic) {this->midPointTopic = midPointTopic;}
        void setCurrentAngle(int angle) {this->currentServoAngle = angle;}

        uint8_t getPinNumber() {return this->pinNumber;}
        char* getPinString() {return this->pinString;}
        const char* getTurnoutTopic() {return this->turnoutTopic;}

        bool initialised = false;

        void loop();

        void messageReceived(receivedMessageEnum message);
        void updateWebPageState();
        void updateWebPageAngle();
        void updatePin(uint8_t newValue);        

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
        char pinString[10];
        Adafruit_PWMServoDriver* pwm = NULL; // NULL means use the GPIO pins on the 8266. If non NULL, then points to the object to drive the pins.
        const char* turnoutTopic;

        int angleThrown = 100;
        int angleClosed = 80;
        int angleMidPoint = 90;
        unsigned long timeFromThrownToClosed_mS = 1000;
        unsigned long timeFromClosedToThrown_mS = 1000;
        const char* thrownTopic = "";
        const char* closedTopic = "";
        const char* midPointTopic = "";

        int currentServoAngle;
        unsigned long movePeriodToClosed_mS;
        unsigned long movePeriodToThrown_mS;
        unsigned long lastTimeServoMoved;

        void calculatePeriods(); 
        void handleStateTransition(stateEnum newState, const char* thrownSensorMessage, const char* closedSensorMessage, const char* midPointMessage);
        const char* stateString(stateEnum state);
        void adjustServoPosition();
        void adjustMovingTowardsClosed();
        void adjustMovingTowardsThrown();
        // void publishMQTTSensor(const char* topic, const char* payload);
        void publishToMQTT(const char* topic, const char* payload);

        void configurePin();
};

#endif
