#include <Arduino.h>
#include <MQTTSensor.h>
//#include <ESP8266WiFi.h>

extern WebSocketsServer webSocket;
extern PubSubClient mqttClient;

MQTTSensor::MQTTSensor(uint8_t pinNumber, const char* sensorTopic, PCF8575* pcf8575) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->sensorTopic = sensorTopic;
    this->pcf8575 = pcf8575;

    configurePin();
}

void MQTTSensor::loop() {
    // Read the sensor pin and publish the appropriate message.
    this->readPin();
}

void MQTTSensor::readPin() {
    int readState;

    // Read the level on pinNumber.
    //readState = digitalRead(this->pinNumber);
    readState = getPin();

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

void MQTTSensor::publishMQTTSensor() {
    char payload[10];

    if (this->currentState == HIGH) {
        strcpy(payload, "INACTIVE");
    } else {
        strcpy(payload, "ACTIVE");
    }

    // Publish the payload to the sensor topic. Retained is set to False.
    mqttClient.publish(this->sensorTopic, payload, false);

    this->updateWebPage();

    Serial.printf("Message published to topic [%s]  %s\n", this->sensorTopic, payload);
}

void MQTTSensor::updateWebPage() {
    // Update the web socket.
    char str[10];

    if (this->currentState == HIGH) {
        sprintf(str,"%i%s",this->pinNumber, "Inactive");
    } else {
        sprintf(str,"%i%s",this->pinNumber, "Active");
    }

    webSocket.broadcastTXT(str);
}

void MQTTSensor::configurePin() {
    if (pcf8575 == NULL) {
        // Using the native pins on the 8266.
        pinMode(this->pinNumber, INPUT_PULLUP);
    } else {
        // Using the I/O expander.

    }
}

int MQTTSensor::getPin() {
    if (pcf8575 == NULL) {
        // Using the native pins on the 8266.
        return digitalRead(this->pinNumber);
    } else {
        // Using the I/O expander.

        return 0; //keep the compiler happy for now!
        
    }
}
