#include "statemachine.h"
#include <string>
#include <sys/time.h>
#include <iostream>

using namespace itpp;
using namespace std;

StateMachine::StateMachine(float lambda, float maxtime, float holdTime, float baseUpperbound) {
    //StateMachine uses the seed value lambda todefine the wait time
    //maxtime: defines the maimum time in any given state
    //holdTime: defines the time a user will have to be in a state
    //baseUpperbound: defines the maximum coordiante to consider a new state
    //baseLowerbound: defines the minimum coordiante to consider a new state

    lambda_ = lambda;
    //maxtime is the maximum alloted time to try and move the cursor
    maxTime_ = maxtime;
    //lambda variable of the exponential distribution
    erng.setup(lambda_);
    // start from center
    _min_wait_time=20.0;
    currentState_ = 0;
    targetState_ = 0;
    inTarget_ = false;
    //hold time is the time the user has to hold in each trial
    holdTime_ = holdTime;
    //baseUpperbound controls the minimum coordinate needed to assign the new state
    baseUpperbound_ = baseUpperbound;
    //waittime is the time to wait before each succesive attempt
    //waitTime_ = erng();
    waitTime_ = _min_wait_time;
    //This assures that the waiting period is over 2 seconds
    gettimeofday(&last_time_, NULL);
}

int StateMachine::UpdateState(float ballpos) {
    timeval now;
    double elapsedTime;
    gettimeofday(&now, NULL);
    elapsedTime = (now.tv_sec - last_time_.tv_sec);
    
    cout<<"currentState_"<<currentState_<<endl;
    cout<<"elapsedTime"<<elapsedTime<<endl;
    cout<<"wait time"<<waitTime_<<endl;
    cout<<"Ball Pos"<<ballpos<<endl;
    cout<<"Ball Pos"<<baseUpperbound_<<endl;
    cout<<"Target Value"<<inTarget_<<endl;
    switch (currentState_) {
    // target is in the resting state
    case 0:
        // if waited long enough go to 1 or -1
        if (elapsedTime > waitTime_) {
            targetState_ = 1;
            gettimeofday(&last_time_, NULL);
        }
        break;

    // target is in the active state
    case 1:


        if (elapsedTime > maxTime_) {
            targetState_ = 0;
            gettimeofday(&last_time_, NULL);
            waitTime_ = _min_wait_time;


        }
        /*
        else if (!inTarget_ && ballpos < baseUpperbound_) {
            inTarget_ = true;
            gettimeofday(&targetEnterTime_, NULL);
        }
        else if (inTarget_ && ballpos > baseUpperbound_) {
            inTarget_ = false;
        }
        else if (inTarget_) {
            //if the object is in target, wait until the ellapsed hold time has passed
            elapsedTime = (now.tv_sec - targetEnterTime_.tv_sec);
            if (elapsedTime > holdTime_) {
                //I_Uniform_RNG rState(-1, 0);
                targetState_ = 0;
                gettimeofday(&last_time_, NULL);
                //This assures that the waiting period is over 2 seconds
                waitTime_ = _min_wait_time;

                }
                inTarget_=false;
            }
*/
        break;


    }

    ostringstream iss;
    iss<<"echo "<<(unsigned char)('b'+targetState_)<<" > /dev/ttyACM0";
    string ttyCmd = iss.str();
    cout<<"ttyCmd "<<ttyCmd<<endl;
    system(ttyCmd.c_str());
    currentState_ = targetState_;
    return targetState_;
}
