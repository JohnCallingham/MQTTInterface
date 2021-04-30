#ifndef MQTT_EEPROM_H
#define MQTT_EEPROM_H

#include <EEPROM.h>

class MQTT_EEPROM {

    public:
        MQTT_EEPROM(){EEPROM.begin(sizeof(eepromContents));};

        bool initialised();
        void initialise();
        //void clear();
        void setServoAngleClosed(uint8_t servoPinNumber, int angle);
        void setServoAngleThrown(uint8_t servoPinNumber, int angle);
        int getServoAngleClosed(uint8_t servoPinNumber);
        int getServoAngleThrown(uint8_t servoPinNumber);

    private:
        struct ServoContents {
            uint8_t pinNumber;
            int angleThrown;
            int angleClosed;
            unsigned long timeFromThrownToClosed_mS;
            unsigned long timeFromClosedToThrown_mS;
            //const char* thrownSensorTopic = "";
            //const char* closedSensorTopic = "";
        };

        struct EEPROMcontents {
            char chipID[10];
            ServoContents servos[16];
            
        };
        EEPROMcontents eepromContents;
};

#endif
