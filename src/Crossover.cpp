#include <Crossover.h>
#include <Servo.h>
#include <MQTTContainer.h>

extern MQTTContainer container;

void Crossover::loop() {

    switch (currentState) {
        case crossoverStateUndefined:
        case crossoverStateThrown:
        case crossoverStateClosed:
            if ((servo1->isStateMoving()) || (servo2->isStateMoving())) {
                // One or both of the servos are moving in one direction or the other.
                Serial.printf("Crossover changed from state '%s' to state '%s'\n", stateString(currentState), stateString(crossoverStateMoving));
                currentState = crossoverStateMoving;

                // Send INACTIVE to both thrown and closed sensor topics.
                container.sendMQTTMessage(this->thrownTopic, "INACTIVE");
                container.sendMQTTMessage(this->closedTopic, "INACTIVE");
            }
            break;

        case crossoverStateMoving:
            // Send ACTIVE to the sensor topic which indicates the destination which has been reached.
            // Both servos need to have reached the same destination for the message to be sent.
            if ((servo1->isStateClosed()) && (servo2->isStateClosed())) {
                // Both servos have reached their closed position.
                Serial.printf("Crossover changed from state '%s' to state '%s'\n", stateString(crossoverStateMoving), stateString(crossoverStateClosed));
                currentState = crossoverStateClosed;
                container.sendMQTTMessage(this->thrownTopic, "INACTIVE");
                container.sendMQTTMessage(this->closedTopic, "ACTIVE");
            }
            if ((servo1->isStateThrown()) && (servo2->isStateThrown())) {
                // Both servos have reached their thrown position.
                Serial.printf("Crossover changed from state '%s' to state '%s'\n", stateString(crossoverStateMoving), stateString(crossoverStateThrown));
                currentState = crossoverStateThrown;
                container.sendMQTTMessage(this->thrownTopic, "ACTIVE");
                container.sendMQTTMessage(this->closedTopic, "INACTIVE");
            }
            break;
    }
}

const char* Crossover::stateString(crossoverState state) {
    switch (state) {
        case crossoverStateUndefined:
            return "Undefined";
        case crossoverStateThrown:
            return "Thrown";
        case crossoverStateMoving:
            return "Moving";
        case crossoverStateClosed:
            return "Closed";
    }
    return "";
}
