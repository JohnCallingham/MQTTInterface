#include <Arduino.h>
#include <MQTTInput.h>
//#include <ESP8266WiFi.h>

extern WebSocketsServer webSocket;
extern PubSubClient mqttClient;

MQTTInput::MQTTInput(uint8_t pinNumber, const char* inputTopic, Adafruit_MCP23017* mcp) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->inputTopic = inputTopic;
    this->mcp = mcp;

    if (this->mcp == NULL) {
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
            publishMQTTInput();
        }
    }

    this->lastState = readState;
}

void MQTTInput::publishMQTTInput() {
    char payload[10];

    if (this->currentState == HIGH) {
        // Publish the default inactive message of "INACTIVE" unless an alternative has been supplied.
        strcpy(payload, this->inactiveMessage);
    } else {
        // Publish the default active message of "ACTIVE" unless an alternative has been supplied.
        strcpy(payload, this->activeMessage);
    }

    // Publish the payload to the input topic. Retained is set to False.
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
    if (mcp == NULL) {
        // Using the native pins on the 8266.
        pinMode(this->pinNumber, INPUT_PULLUP);
    } else {
        // Using the I/O expander.
        mcp->pinMode(this->pinNumber, INPUT);  //crashes here !!! need to call mcp->begin() first !!!
        mcp->pullUp(this->pinNumber, HIGH);  // turn on a 100K pullup internally
    }
}

int MQTTInput::getPin() {
    if (mcp == NULL) {
        // Using the native pins on the 8266.
        return digitalRead(this->pinNumber);
    } else {
        // Using the I/O expander.
        return mcp->digitalRead(this->pinNumber);
    }
}
