#include <Arduino.h>
#include <MQTTInput.h>
//#include <ESP8266WiFi.h>

extern WebSocketsServer webSocket;
extern PubSubClient mqttClient;

MQTTInput::MQTTInput(uint8_t pinNumber, const char* inputTopic, PCF8575* pcf8575) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->inputTopic = inputTopic;
    this->pcf8575 = pcf8575;

    if (this->pcf8575 == NULL) {
        sprintf(this->pinString, "N%02i", this->pinNumber);
        sprintf(this->pinID, "N%02i", this->pinNumber);
    } else {
        sprintf(this->pinString, "X%02i", this->pinNumber);
        sprintf(this->pinID, "X%02i", this->pinNumber + 16);
    }

    configurePin();
}

void MQTTInput::loop() {
    // Read the sensor pin and publish the appropriate message.
    this->readPin();
}

void MQTTInput::readPin() {
    int readState;

    // Read the level on pinNumber.
    //readState = digitalRead(this->pinNumber);
    readState = getPin();

    //Serial.println(readState);

    // Has the pin level changed due to bounce or otherwise?
    if (readState != this->lastState) {
        // Yes, so reset the debounce timer.
        this->lastTime = millis();
    }

    // Has time moved on from the last level change by the debounce delay?
    if ((millis() - this->lastTime) > this->debounceDelay_mS) {
        // Whatever the current level, it has been stable for the debounce delay.
        // So check for a change in level.
        if (readState != this->currentState) {
            // There has been a debounced change in level so store the new level.
            this->currentState = readState;

            // Publish the change to the MQTT broker.
            publishMQTTSensor();
        }
    }

    this->lastState = readState;
}

void MQTTInput::publishMQTTSensor() {
    char payload[10];

    if (this->currentState == HIGH) {
        // Use the default message if inactiveMessage is not set.
        // strcpy(payload, "INACTIVE");
        strcpy(payload, this->inactiveMessage);
    } else {
        // Use the default message if activeMessage is not set.
        // strcpy(payload, "ACTIVE");
        strcpy(payload, this->activeMessage);
    }

    // Publish the payload to the sensor topic. Retained is set to False.
    // If this input is connected to a button which positions a turnout, then there will not be an inactive mesasge.
    if (strlen(payload) > 0) {
        mqttClient.publish(this->inputTopic, payload, false);
    }

    this->updateWebPage();

    Serial.printf("Message published to topic [%s]  %s\n", this->inputTopic, payload);
}

void MQTTInput::updateWebPage() {
    // Update the web socket.
    char str[10];

    if (this->currentState == HIGH) {
        // sprintf(str,"s%s%s",this->pinString, "Inactive");
        sprintf(str,"s%s%s",this->pinID, "Inactive");
    } else {
        // sprintf(str,"s%s%s",this->pinString, "Active");
        sprintf(str,"s%s%s",this->pinID, "Active");
    }

    webSocket.broadcastTXT(str);
}

void MQTTInput::configurePin() {
    if (pcf8575 == NULL) {
        // Using the native pins on the 8266.
        pinMode(this->pinNumber, INPUT_PULLUP);
    } else {
        // Using the I/O expander.
        pcf8575->pinMode(this->pinNumber, INPUT);
    }
}

int MQTTInput::getPin() {
    if (pcf8575 == NULL) {
        // Using the native pins on the 8266.
        return digitalRead(this->pinNumber);
    } else {
        // Using the I/O expander.
        //return pcf8575->digitalRead(pinNumber); // or use digitalInput() ??? causing repeated output of 0 to the serial port

        return 0; //keep the compiler happy for now!
        
    }
}
