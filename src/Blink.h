#ifndef BLINK_H
#define BLINK_H

class Blink {

    public:

        Blink(unsigned long onTime_mS, unsigned long offTime_mS);

        void loop();

        bool ledStateOn = false;

    private:

        unsigned long timeToTurnLEDOff = 0;
        unsigned long timeToTurnLEDOn = 0;

        unsigned long onTime_mS;
        unsigned long offTime_mS;

};

#endif
