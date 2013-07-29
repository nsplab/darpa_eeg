#include "eeg_receiver.h"
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

EegReceiver::EegReceiver(size_t numChannels_, size_t numScans_):
  context(1), eeg_subscriber(context, ZMQ_SUB), numChannels(numChannels_), numScans(numScans_)
{
    eeg_subscriber.connect("tcp://192.168.56.101:5556");
    eeg_subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

}

void EegReceiver::receive(double& time, float* channels) {
    zmq::message_t update;
    //eeg_subscriber.recv(&update, ZMQ_NOBLOCK);
    eeg_subscriber.recv(&update);
    std::istringstream iss(static_cast< char*>(update.data()));

    //cout<<update.size()<<endl;
    memcpy(&time, update.data(), sizeof(double));
    memcpy(channels, (char*)update.data()+sizeof(double), sizeof(float) * numChannels * numScans);

    /*std::string str;
    int count = 0;
    while (iss>>str) {
      cout<<" "<<count<<":"<<str;
      count+=1;
      }*/
    //cout<<endl;
    /*for (int i=0; i<65; i++) {
        iss >> str;
        channels[i] = atof(str.c_str());
    }*/
    //iss >> str;
    //channels[64] = atoi(str.c_str());

}
