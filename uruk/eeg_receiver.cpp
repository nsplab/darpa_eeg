#include "eeg_receiver.h"
#include <sstream>
#include <string>

EegReceiver::EegReceiver():
    context(1), eeg_subscriber(context, ZMQ_SUB)
{
    eeg_subscriber.connect("tcp://192.168.56.101:5556");
    eeg_subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

}

void EegReceiver::receive(float* channels) {
    zmq::message_t update;
    eeg_subscriber.recv(&update, ZMQ_NOBLOCK);
    //eeg_subscriber.recv(&update);
    std::istringstream iss(static_cast< char*>(update.data()));

    std::string str;
    for (int i=0; i<64; i++) {
        iss >> str;
        channels[i] = atof(str.c_str());
    }
    iss >> str;
    channels[64] = atoi(str.c_str());

}
