#include "statemachine.h"
#include <string>
#include <sys/time.h>

using namespace itpp;
using namespace std;

StateMachine::StateMachine(float lambda, float maxtime, float holdTime, float baseUpperbound, float baseLowerbound) {
    lambda_ = lambda;
    maxTime_ = maxtime;
    erng.setup(lambda_);
    // start from center
    currentState_ = 0;
    targetState_ = 0;
    inTarget_ = false;
    holdTime_ = holdTime;
    baseUpperbound_ = baseUpperbound;
    baseLowerbound_ = baseLowerbound;
    waitTime_ = erng();
}

int StateMachine::UpdateState(float ballpos) {
    timeval now;
    double elapsedTime;
    gettimeofday(&now, NULL);
    elapsedTime = (now.tv_sec - last_time_.tv_sec);

    switch (currentState_) {
    // target is center
    case 0:
        // if waited long enough go to 1 or -1
        if (elapsedTime > waitTime_) {
            I_Uniform_RNG rState(0, 1);
            targetState_ = (rState() == 1)? 1:-1;
            gettimeofday(&last_time_, NULL);
        }
        break;

    // target is right
    case 1:

        if (elapsedTime > maxTime_) {
            targetState_ = 0;
            gettimeofday(&last_time_, NULL);
            waitTime_ = erng();
        }
        if (!inTarget_ && ballpos > baseUpperbound_) {
            inTarget_ = true;
            gettimeofday(&targetEnterTime_, NULL);
        }
        if (inTarget_ && ballpos < baseUpperbound_) {
            inTarget_ = false;
        }
        if (inTarget_) {
            elapsedTime = (now.tv_sec - targetEnterTime_.tv_sec);
            if (elapsedTime > holdTime_) {
                I_Uniform_RNG rState(-1, 0);
                targetState_ = rState();
                gettimeofday(&last_time_, NULL);
            }
        }
        break;

    // target is left
    case -1:
        if (elapsedTime > maxTime_) {
            targetState_ = 0;
            gettimeofday(&last_time_, NULL);
            waitTime_ = erng();
        }
        if (!inTarget_ && ballpos < baseLowerbound_) {
            inTarget_ = true;
            gettimeofday(&targetEnterTime_, NULL);
        }
        if (inTarget_ && ballpos > baseLowerbound_) {
            inTarget_ = false;
        }
        if (inTarget_) {
            elapsedTime = (now.tv_sec - targetEnterTime_.tv_sec);
            if (elapsedTime > holdTime_) {
                I_Uniform_RNG rState(0, -1);
                targetState_ = rState();
                gettimeofday(&last_time_, NULL);
            }
        }
        break;
    }

    string ttyCmd = "echo "+to_string('b'+targetState_)+" > /dev/ttyACM0";
    system(ttyCmd.c_str());
    currentState_ = targetState_;
    return targetState_;
}
