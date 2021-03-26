/*
Basic - subscribes to a single topic - ACTIVE operates the relay, INACTIVE releases the relay.
Advanced - subscribes to two topics - one topic ACTIVE operates the realy, the other topic ACTIVE releases the relay.
*/
#include <MQTTRelay.h>
#include <WebSocketsServer.h>

extern WebSocketsServer webSocket;

MQTTRelay::MQTTRelay(uint8_t pinNumber, const char* relayTopic, PCF8575* pcf8575) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->relayTopic = relayTopic;
    this->pcf8575 = pcf8575;

    configurePin();

    // Initialise to released.
    strcpy(this->currentState, "Released");
}

MQTTRelay::MQTTRelay(uint8_t pinNumber, const char* relayOperateTopic, const char* relayReleaseTopic, PCF8575* pcf8575) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->relayOperateTopic = relayOperateTopic;
    this->relayReleaseTopic = relayReleaseTopic;
    this->pcf8575 = pcf8575;

    configurePin();

    // Initialise to released.
    strcpy(this->currentState, "Released");
}

void MQTTRelay::receivedRelayTopic(char* payload) {
    // JMRI sensors send "ACTIVE" or "INACTIVE". JMRI lights send "ON" or "OFF".
    // If payload == "ACTIVE" or "ON", operate relay.
    // If payload == "INACTIVE" or "OFF", release relay.
    if ((strcmp(payload, "ACTIVE") == 0) || (strcmp(payload, "ON")) == 0) {
        Serial.printf("Basic relay on pin %i operated\n", this->pinNumber);
        // digitalWrite(this->pinNumber, LOW);
        updatePin(LOW);
        strcpy (this->currentState, "Operated");
        updateWebPage();
    }
    if ((strcmp(payload, "INACTIVE") == 0) || (strcmp(payload, "OFF")) == 0) {
        Serial.printf("Basic relay on pin %i released\n", this->pinNumber);
        // digitalWrite(this->pinNumber, HIGH);
        updatePin(HIGH);
        strcpy (this->currentState, "Released");
        updateWebPage();
    }
}

void MQTTRelay::receivedRelayOperateTopic(char* payload) {
    // If payload == "ACTIVE", operate relay.
    // If payload == "INACTIVE", do nothing.
    if (strcmp(payload, "ACTIVE") == 0) {
        Serial.printf("Advanced relay on pin %i operated\n", this->pinNumber);
        updatePin(LOW);
        strcpy (this->currentState, "Operated");
        updateWebPage();
    }
}

void MQTTRelay::receivedRelayReleaseTopic(char* payload) {
    // If payload == "ACTIVE", release relay.
    // If payload == "INACTIVE", do nothing.
    if(strcmp(payload, "ACTIVE") == 0) {
        Serial.printf("Advanced relay on pin %i released\n", this->pinNumber);
        updatePin(HIGH);
        strcpy (this->currentState, "Released");
        updateWebPage();
    }
}

void MQTTRelay::updateWebPage() {
    // Update the web socket.
    char str[60];

    sprintf(str, "%i%s", this->pinNumber, this->currentState);

    webSocket.broadcastTXT(str);
}

void MQTTRelay::configurePin() {
    if (pcf8575 == NULL) {
        // Using the native pins on the 8266.
        pinMode(this->pinNumber, OUTPUT);
    } else {
        // Using the I/O expander.

    }
}

void MQTTRelay::updatePin(uint8_t newValue) {
    if (pcf8575 == NULL) {
        // Using the native pins on the 8266.
        digitalWrite(this->pinNumber, newValue);
    } else {
        // Using the I/O expander.
        
    }
}
