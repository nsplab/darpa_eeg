#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <itpp/base/random.h>

class StateMachine {
public:
    StateMachine(float lambda, float maxtime, float holdTime, float baseUpperbound);
    int UpdateState(float ballpos);

private:
    int targetState_;
    int currentState_;
    float lambda_;
    float maxTime_;
    float waitTime_;
    double _min_wait_time;
    // time ball enters target region
    timeval targetEnterTime_;
    float holdTime_;
    bool inTarget_;
    //Exponential random generator
    itpp::Exponential_RNG erng;
    timeval last_time_;
    float baseUpperbound_;
};

#endif // STATEMACHINE_H
