#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <itpp/base/random.h>

class StateMachine {
public:
    StateMachine(float lambda, float maxtime, float holdTime, float baseUpperbound, float baseLowerbound);
    int UpdateState(float ballpos);

private:
    int targetState_;
    int currentState_;
    float lambda_;
    float maxTime_;
    float waitTime_;
    // time ball enters target region
    timeval targetEnterTime_;
    float holdTime_;
    bool inTarget_;
    itpp::Exponential_RNG erng;
    timeval last_time_;
    float baseUpperbound_;
    float baseLowerbound_;
};

#endif // STATEMACHINE_H
