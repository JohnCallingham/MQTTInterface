/*
ACTIVE or ON turns the output on.
INACTIVE or OFF turns the output off.
*/
#include <MQTTOutput.h>
#include <WebSocketsServer.h>

extern WebSocketsServer webSocket;

MQTTOutput::MQTTOutput(uint8_t pinNumber, const char* outputTopic, Adafruit_MCP23017* mcp) {
    // Store the parameters.
    this->pinNumber = pinNumber;
    this->outputTopic = outputTopic;
    this->mcp = mcp;

    if (this->mcp == NULL) {
        sprintf(this->pinString, "N%02i", this->pinNumber);
        sprintf(this->pinID, "N%02i", this->pinNumber);
    } else {
        sprintf(this->pinString, "X%02i", this->pinNumber);
        sprintf(this->pinID, "X%02i", this->pinNumber + 16);
    }

    configurePin();

    // Initialise to released.
    strcpy(this->currentState, "Off");
}

void MQTTOutput::messageReceived(char* payload) {
    // JMRI sensors send "ACTIVE" or "INACTIVE". JMRI lights send "ON" or "OFF".
    // If payload == "ACTIVE" or "ON", operate relay.
    // If payload == "INACTIVE" or "OFF", release relay.
    if ((strcmp(payload, "ACTIVE") == 0) || (strcmp(payload, "ON")) == 0) {
        Serial.printf("Output on pin %i on\n", this->pinNumber);
        updatePin(LOW);
        strcpy (this->currentState, "On");
        updateWebPage();
    }
    if ((strcmp(payload, "INACTIVE") == 0) || (strcmp(payload, "OFF")) == 0) {
        Serial.printf("Output on pin %i off\n", this->pinNumber);
        updatePin(HIGH);
        strcpy (this->currentState, "Off");
        updateWebPage();
    }
}

void MQTTOutput::updateWebPage() {
    // Update the web socket.
    char str[60];

    // sprintf(str, "%i%s", this->pinNumber, this->currentState);
    // sprintf(str, "s%s%s", this->pinString, this->currentState);
    sprintf(str, "s%s%s", this->pinID, this->currentState);

    webSocket.broadcastTXT(str);
}

void MQTTOutput::configurePin() {
    if (mcp == NULL) {
        // Using the native pins on the 8266.
        pinMode(this->pinNumber, OUTPUT);
    } else {
        // Using the I/O expander.

    }
}

void MQTTOutput::updatePin(uint8_t newValue) {
    if (mcp == NULL) {
        // Using the native pins on the 8266.
        digitalWrite(this->pinNumber, newValue);
    } else {
        // Using the I/O expander.
        
    }
}
