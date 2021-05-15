#ifndef CROSSOVER_H
#define CROSSOVER_H

#include <MQTTServo.h>

class Crossover {

    public:
        Crossover(MQTTServo* servo1, MQTTServo* servo2) {this->servo1 = servo1; this->servo2 = servo2;}

        void setThrownTopic(const char* thrownTopic) {this->thrownTopic = thrownTopic;}
        void setClosedTopic(const char* closedTopic) {this->closedTopic = closedTopic;}
        void setMidPointTopic(const char* midPointTopic) {this->servo1->setMidPointTopic(midPointTopic);}

        void loop();

    private:
        MQTTServo* servo1;
        MQTTServo* servo2;
      
        const char* thrownTopic = "";
        const char* closedTopic = "";

        enum crossoverState {
            crossoverStateUndefined,
            crossoverStateThrown,
            crossoverStateClosed,
            crossoverStateMoving
        };
        crossoverState currentState = crossoverStateUndefined;

        const char* stateString(crossoverState state);

};

#endif
