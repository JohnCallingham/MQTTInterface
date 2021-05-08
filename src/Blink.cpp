#include <Blink.h>
#include <Arduino.h>

Blink::Blink(unsigned long onTime_mS, unsigned long offTime_mS) {
    this->onTime_mS = onTime_mS;
    this->offTime_mS = offTime_mS;
}

void Blink::loop() {

    if (this->ledStateOn) {
        // The LED is on.
        if (millis() >= timeToTurnLEDOff) {
            this->ledStateOn = false;
            timeToTurnLEDOn = millis() + offTime_mS;
        }
    } else {
        // The LED is off.
        if (millis() >= timeToTurnLEDOn) {
            this->ledStateOn = true;
            timeToTurnLEDOff = millis() + onTime_mS;
        }
    }
}
