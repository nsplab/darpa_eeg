#ifndef EEG_RECEIVER_H
#define EEG_RECEIVER_H

#include <algorithm>
#include <zmq.hpp>
#include <vector>

class EegReceiver
{
public:
    EegReceiver(size_t numChannels_, size_t numScans_);
    void receive(double& time, float* channels);
private:
    zmq::context_t context;
    zmq::socket_t eeg_subscriber;
    size_t numChannels;
    size_t numScans;
};

#endif // EEG_RECEIVER_H
