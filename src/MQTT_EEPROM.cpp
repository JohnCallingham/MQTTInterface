#include <MQTT_EEPROM.h>
#include <Arduino.h>
#include <EEPROM.h>

bool MQTT_EEPROM::initialised() {
    char chipID[10];
    sprintf(chipID, "%04x", ESP.getChipId());

    // Load EEPROM into eepromContents.
    EEPROM.get(0, eepromContents);

    Serial.printf("ChipID: %s.\n", eepromContents.chipID);

    if (strcmp(eepromContents.chipID, chipID) == 0) {
        Serial.println("EEPROM initialised.");
        return true;
    } else {
        Serial.println("EEPROM not initialised.");
        return false;
    }
}

void MQTT_EEPROM::initialise() {

    // Set all sensor default values.

    // Set all relay default values.

    // Set all servo default values.
    for (int i=0; i<16; i++) {
        eepromContents.servos[i].pinNumber = i;
        eepromContents.servos[i].angleClosed = 80;
        eepromContents.servos[i].angleThrown = 100;
        eepromContents.servos[i].timeFromClosedToThrown_mS = 1000;
        eepromContents.servos[i].timeFromThrownToClosed_mS = 1000;
    }

    // Set the EEPROM as initialised.
    sprintf(eepromContents.chipID, "%04x", ESP.getChipId());

    // Write all values to the EEPROM.
    EEPROM.put(0, eepromContents);
    EEPROM.commit();

    Serial.println("EEPROM initialised.");

}

void MQTT_EEPROM::setServoAngleClosed(uint8_t servoPinNumber, int angle) {
    eepromContents.servos[servoPinNumber].angleClosed = angle;
    EEPROM.put(0, eepromContents);
    EEPROM.commit();
    Serial.printf("setServoAngleClosed: %i\n", angle);
}

void MQTT_EEPROM::setServoAngleThrown(uint8_t servoPinNumber, int angle) {
    eepromContents.servos[servoPinNumber].angleThrown = angle;
    EEPROM.put(0, eepromContents);
    EEPROM.commit();
    Serial.printf("setServoAngleThrown: %i\n", angle);
}

int MQTT_EEPROM::getServoAngleClosed(uint8_t servoPinNumber) {
    EEPROM.get(0, eepromContents);
    Serial.printf("getServoAngleClosed: %i\n", eepromContents.servos[servoPinNumber].angleClosed);
    return eepromContents.servos[servoPinNumber].angleClosed;
}

int MQTT_EEPROM::getServoAngleThrown(uint8_t servoPinNumber) {
    EEPROM.get(0, eepromContents);
    Serial.printf("getServoAngleThrown: %i\n", eepromContents.servos[servoPinNumber].angleThrown);
    return eepromContents.servos[servoPinNumber].angleThrown;
}
