#ifndef EEG_RECEIVER_H
#define EEG_RECEIVER_H

#include <zmq.hpp>

class EegReceiver
{
public:
    EegReceiver();
    void receive(float* channels);
private:
    zmq::context_t context;
    zmq::socket_t eeg_subscriber;
};

#endif // EEG_RECEIVER_H
