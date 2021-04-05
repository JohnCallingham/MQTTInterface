#include <Arduino.h>
#include <MQTTServo.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <MQTT_EEPROM.h>

extern WebSocketsServer webSocket;
extern PubSubClient mqttClient;
extern MQTT_EEPROM mqttEEPROM;

MQTTServo::MQTTServo(uint8_t pinNumber, const char* turnoutTopic, Adafruit_PWMServoDriver* pwm) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->turnoutTopic = turnoutTopic;
    this->pwm = pwm;

    if (this->pwm == NULL) {
        sprintf(this->pinString, "N%02i", this->pinNumber);
    } else {
        sprintf(this->pinString, "X%02i", this->pinNumber);
    }

    // Configure the servo pin.
    configurePin();
}

void MQTTServo::loop() {
    if (!initialised) {
        calculatePeriods();

        // Load values from EEPROM.
        if (mqttEEPROM.initialised()) {
            Serial.println("Getting angles for servo.");
            this->setAngleClosed(mqttEEPROM.getServoAngleClosed(pinNumber));
            this->setAngleThrown(mqttEEPROM.getServoAngleThrown(pinNumber));
        }

        initialised = true;
    }

    // Check if the servo position needs updating.
    adjustServoPosition();
}

void MQTTServo::messageReceived(receivedMessageEnum message) {
    switch (currentState) {
        case stateUndefined:
            // Start the servo midway between closed and thrown.
            // Accomodates either the thrown or closed angle being the largest.
            if (this->angleThrown == this->angleClosed) {
                this->currentServoAngle = this->angleThrown;
            } else if (this->angleThrown > this->angleClosed) {
                this->currentServoAngle = this->angleClosed + (abs(this->angleClosed - this->angleThrown)/2);
            } else {
                this->currentServoAngle = this->angleThrown + (abs(this->angleClosed - this->angleThrown)/2);
            }
            //Serial.println(this->currentServoAngle);

            switch(message) {
                case messageThrown:
                    handleStateTransition(stateMoving_Towards_Thrown, "INACTIVE", "INACTIVE");
                    break;
                case messageClosed:
                    handleStateTransition(stateMoving_Towards_Closed, "INACTIVE", "INACTIVE");
                    break;
                case reachedThrown:
                    // Error.
                    break;
                case reachedClosed:
                    // Error.
                    break;
            }
            break;
        case stateThrown:
            switch(message) {
                case messageThrown:
                    handleStateTransition(stateThrown, "ACTIVE", "INACTIVE");
                    break;
                case messageClosed:
                    handleStateTransition(stateMoving_Towards_Closed, "INACTIVE", "INACTIVE");
                    break;
                case reachedThrown:
                    // Error.
                    break;
                case reachedClosed:
                    // Error.
                    break;
            }
            break;
        case stateMoving_Towards_Closed:
            switch(message) {
                case messageThrown:
                    handleStateTransition(stateMoving_Towards_Thrown, "INACTIVE", "INACTIVE");
                    break;
                case messageClosed:
                    handleStateTransition(stateMoving_Towards_Closed, "INACTIVE", "INACTIVE");
                    break;
                case reachedThrown:
                    // Error.
                    break;
                case reachedClosed:
                    handleStateTransition(stateClosed, "INACTIVE", "ACTIVE");
                    break;
            }
            break;
        case stateClosed:
            switch(message) {
                case messageThrown:
                    handleStateTransition(stateMoving_Towards_Thrown, "INACTIVE", "INACTIVE");
                    break;
                case messageClosed:
                    handleStateTransition(stateClosed, "INACTIVE", "ACTIVE");
                    break;
                case reachedThrown:
                    // Error.
                    break;
                case reachedClosed:
                    // Error.
                    break;
            }
            break;
        case stateMoving_Towards_Thrown:
            switch(message) {
                case messageThrown:
                    handleStateTransition(stateMoving_Towards_Thrown, "INACTIVE", "INACTIVE");
                    break;
                case messageClosed:
                    handleStateTransition(stateMoving_Towards_Closed, "INACTIVE", "INACTIVE");
                    break;
                case reachedThrown:
                    handleStateTransition(stateThrown, "ACTIVE", "INACTIVE");
                    break;
                case reachedClosed:
                    // Error.
                    break;
            }
            break;
    }
}

void MQTTServo::handleStateTransition(stateEnum newState, const char* thrownSensorMessage, const char* closedSensorMessage) {
    // Serial.printf("State changed from '%i' to '%i'\n", currentState, newState);
    Serial.printf("Servo on pin %s state changed from '%s' to '%s'\n", this->pinString, stateString(this->currentState), stateString(newState));

    this->currentState = newState;

    publishMQTTSensor(thrownSensorTopic, thrownSensorMessage);
    publishMQTTSensor(closedSensorTopic, closedSensorMessage);

    updateWebPageState();
}

void MQTTServo::updateWebPageState() {
    // Update the web socket.
    char strState[60];

    sprintf(strState, "s%s%s", this->pinString, stateString(this->currentState));
    webSocket.broadcastTXT(strState);
}

void MQTTServo::updateWebPageAngle() {
    // Update the web socket.
    char strAngle[60];

    sprintf(strAngle, "r%s%i", this->pinString, this->currentServoAngle);
    webSocket.broadcastTXT(strAngle);
}

const char* MQTTServo::stateString(stateEnum state) {
    switch (state) {
        case stateUndefined:
            return "Undefined";
        case stateThrown:
            return "Thrown";
        case stateMoving_Towards_Closed:
            return "Moving towards closed";
        case stateClosed:
            return "Closed";
        case stateMoving_Towards_Thrown:
            return "Moving towards thrown";
    }
    return "";
}

void MQTTServo::adjustServoPosition() {
    // Checks to see if it is time to make an adjustment to the servo position.

    // If the servo is moving then check if it is time to make another move.
    switch (this->currentState) {
        case stateMoving_Towards_Closed:
            adjustMovingTowardsClosed();
            break;
        case stateMoving_Towards_Thrown:
            adjustMovingTowardsThrown();
            break;
        default:
            // Do nothing.
            break;
    }
}

void MQTTServo::adjustMovingTowardsClosed() {
    // Is it time to make another movement?
    if ((millis() - this->lastTimeServoMoved) > this->movePeriodToClosed_mS) {
        // Yes, so update the servo angle, unless we are already there.
        if (this->currentServoAngle != this->angleClosed) {
            if (this->angleThrown > this->angleClosed) {
                if (this->currentServoAngle > 0) {
                    this->currentServoAngle--;
                    updateWebPageAngle();
                }
            } else {
                if (this->currentServoAngle < 180) {
                    this->currentServoAngle++;
                    updateWebPageAngle();
                }
            }
        }

        this->lastTimeServoMoved = millis();

        // Have we reached the end angle?
        // if (this->currentServoAngle > this->angleClosed) {
        if (this->currentServoAngle == this->angleClosed) {
            // Yes, so send the appropriate MQTT message and change the current state.
            messageReceived(reachedClosed);
            return;
        } else {
            // Not yet, so move the servo.
            //Serial.printf ("Servo on pin %s, Turnout %s, Servo angle %i\n", this->pinString, turnoutTopic, currentServoAngle);
            updatePin(currentServoAngle);
        }
    }
}

void MQTTServo::adjustMovingTowardsThrown() {
    // Is it time to make another movement?
    if ((millis() - this->lastTimeServoMoved) > this->movePeriodToThrown_mS) {
        // Yes, so update the servo angle, unless we are already there.
        if (this->currentServoAngle != this->angleThrown) {
            if (this->angleThrown > this->angleClosed) {
                if (this->currentServoAngle < 180) {
                    this->currentServoAngle++;
                    updateWebPageAngle();
                }
            } else {
                if (this->currentServoAngle > 0) {
                    this->currentServoAngle--;
                    updateWebPageAngle();
                }
            }
        }

        this->lastTimeServoMoved = millis();

        // Have we reached the end angle?
        // if (this->currentServoAngle < this->angleThrown) {
        if (this->currentServoAngle == this->angleThrown) {
            // Yes, so send the appropriate MQTT message and change the current state.
            messageReceived(reachedThrown);
            return;
        } else {
            // Not yet, so move the servo.
            //Serial.printf ("Servo on pin %s, Turnout %s, Servo angle %i\n", this->pinString, turnoutTopic, currentServoAngle);
            updatePin(currentServoAngle);
        }
    }
}

void MQTTServo::calculatePeriods() {
    // Calculates how often to move the servo to achieve a complete transition from one state to another in timeFrom..._mS.
    // Accomodates either the thrown or closed angle being the largest.
    if (this->angleClosed == this->angleThrown) {
        movePeriodToClosed_mS = timeFromThrownToClosed_mS / 50; // TO DO sort a more sensible value.
        movePeriodToThrown_mS = timeFromClosedToThrown_mS / 50; // TO DO still goes to 180 when setting both T and C !!!
    } else {
        movePeriodToClosed_mS = timeFromThrownToClosed_mS/abs(angleClosed - angleThrown);
        movePeriodToThrown_mS = timeFromClosedToThrown_mS/abs(angleClosed - angleThrown);
    }
}

void MQTTServo::publishMQTTSensor(const char* topic, const char* payload) {
    // Do not publish if the topic is empty.
    if (strlen(topic) != 0) {
        // Publish the payload to the sensor topic. Retained is set to False.
        mqttClient.publish(topic, payload, false);

        Serial.printf("Message published to topic [%s]  %s\n", topic, payload);
    }
}

void MQTTServo::configurePin() {
    if (this->pwm == NULL) {
        // Using the native pins on the 8266.
        pinMode(this->pinNumber, OUTPUT);
    } else {
        // Using the I/O expander.

    }
}

void MQTTServo::updatePin(uint8_t newValue) {
    if (this->pwm == NULL) {
        // Using the native pins on the 8266.
        // This has not been impelmented.
    } else {
        // Using the I/O expander.

        // Convert angle to off position out of 4096.
        // 0 degrees is 110. 180 degrees is 500.

        //val = map(val, 0, 180, 110, 500);
        this->pwm->setPWM(this->pinNumber, 0, map(newValue, 0, 180, 110, 500));

        
    }
}
