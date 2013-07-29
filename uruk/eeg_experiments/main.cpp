#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <thread>
#include <fstream>
#include <sstream>

#include <vector>
#include <iomanip> // setprecision
#include <chrono> // timing
#include <sys/resource.h>

#include <SDL/SDL.h>
#include <algorithm>
#include <zmq.hpp>

#include "utils.h"
#include "gnuplot.h"
#include "eeg_receiver.h"
#include "fft.h"


using namespace std;
using namespace chrono;
using namespace zmq;

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int main()
{
    //Code to manage the interactivity with python
    context_t context_py(3);
    socket_t publisher(context_py, ZMQ_PUB);
    socket_t subscriber(context_py, ZMQ_SUB);
    publisher.bind("tcp://127.0.0.1:50001");
    //publisher.setsockopt(ZMQ_PUB,NULL,0);
    subscriber.connect("tcp://127.0.0.1:50000");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    stringstream message;

    // change the proiority and scheduler of process
    /*struct sched_param param;
    param.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_FIFO, & param) != 0) {
      perror("sched_setscheduler");
      exit(EXIT_FAILURE);
    }*/

    // data to be received
    double time;
    size_t numChannels = 65;
    size_t numChannelsWOT = numChannels - 1;
    size_t numScans = 32;
    size_t dataSize = numChannels * numScans;
    float channels[dataSize];
    EegReceiver eeg(numChannels, numScans);

    // save received data into file
    FILE* pFile;
    pFile = fopen("data", "wb");
    setvbuf (pFile, NULL, _IOFBF, dataSize*sizeof(float));
    /*This chunk was from the new format file*/

    size_t sampling_frq = 4800; // Hz
    size_t dft_points = 1024; // points

  cout<<"l0"<<endl;
    Fft<double> fft(dft_points, Fft<double>::windowFunc::HAMMING, sampling_frq, numChannelsWOT);
  cout<<"l1"<<endl;
    vector<vector<double> > powers(numChannels);
    vector<vector<double> > phases(numChannels);
    for (size_t i=0; i<numChannels; i++) {
      powers[i].resize(dft_points/2+1);
      phases[i].resize(dft_points/2+1);
    }
    vector<double> point(numChannelsWOT);

    struct timespec requestStart, requestEnd;

    // alpha value in first order autoregressive
    float alpha = 0.75;

    // signals gui window to close
    bool exit = false;

    // the ball/cursor whose vertical position(redRect.y) is controlled by EEG
    SDL_Rect redRect;
    redRect.x = 960;
    redRect.y = 500;
    redRect.h = 50; // height
    redRect.w = 50; // width
    float reward_money=0.0; //Money Reward to be displayed in the screen

    const size_t numChannelsPlt = 4;
    vector<float> pwrPlt(numChannelsPlt);
    vector<string> legends;
    legends.push_back(string("ch 1"));
    legends.push_back(string("ch 2"));
    legends.push_back(string("ch 3"));
    legends.push_back(string("ch 4"));
    // gnuplot window to plot live data
    GnuPlot gnuplot(legends);
    // * SDL - graphics
    // register exit function of SDL to release taken memory gracefully
    atexit(SDL_Quit);
    // title of window
    SDL_WM_SetCaption("Uruk - NSPLab", NULL);
    // create window of 800 by 480 pixels
    SDL_Surface* screen = SDL_SetVideoMode( 1920, 1200, 32, SDL_DOUBLEBUF|SDL_ANYFORMAT);
    SDL_ShowCursor(SDL_DISABLE);
    // dialog box to modify alpha
    thread gui(runGUI, &alpha, &exit);

    // receive EEG signal using ZMQ

    //relax_csv<<baselineSamples<<endl;
    //left_relax_csv<<baselineSamples_left<<endl;
    //right_relax_csv<<baselineSamples_right<<endl;

    int loop = 0;
    //StateMachine stateMachine(0.4, 5.0, 5.0, 425);    //stateMachine arguments are (lambda, maxtime, holdtime, baseupperbound, baselowerbound)
    double alpha_0=20.0;
    double alpha_1=2.0;
    double alpha_2=1.0;
    double alpha_sat;
    double beta_1=525;
    double beta_2=325;
    double beta_0=625;
    int rest_counter=0;
    int active_counter=0;
    double y_position=0.0;
    int target = 0;
    sleep(5);
    //625 and 425 are the graphical lower and upper bounds respectively, if the boxes are shifted
    //these numbers have to be changed
    //*********************************************************************
    //*********************************************************************
    //** Experiment
    //In the experiment, we have already calculated the upper and lower bounds
    //for the baseline.
    //*********************************************************************
    //*********************************************************************
    // main loop
    timeval init_time;
    gettimeofday(&init_time, NULL);
     //counter holds the register to control how many samples to wait until calculating a new spectrum
      for(; !kbhit();){
         eeg.receive(time, channels);
         cout<<fixed<<setprecision(9)<<time<<endl;

        // receive EEG
         for (size_t i=0; i<numScans; i++) {
           for (size_t ch=0; ch<numChannelsWOT; ch++) {
             point[ch] = channels[i*numChannels+ch];
           }
           // large laplacians
           point[62] = channels[27] - 0.25 * (channels[25]+channels[9]+channels[29]+channels[45]);
           point[63] = channels[31] - 0.25 * (channels[29]+channels[13]+channels[33]+channels[49]);
           fft.AddPoints(point);
         }

         loop += 1;
         if (loop > 10) {
           clock_gettime(CLOCK_REALTIME, &requestStart);
           if (fft.Process()) {
             fft.GetPower(powers);
             pwrPlt[0] = (alpha) * pwrPlt[0] + (1.0 - alpha) * (powers[27][3] - 0.25 * (powers[25][3]+powers[9][3]+powers[29][3]+powers[45][3]));
             pwrPlt[1] = (alpha) * pwrPlt[1] + (1.0 - alpha) * (powers[31][3] - 0.25 * (powers[29][3]+powers[13][3]+powers[33][3]+powers[49][3]));
             pwrPlt[2] = (alpha) * pwrPlt[2] + (1.0 - alpha) * powers[62][3];
             pwrPlt[3] = (alpha) * pwrPlt[3] + (1.0 - alpha) * powers[63][3];

             float mean_power = pwrPlt[2];
                    //- left_av_mu_power;
             // publish computed powers using ZMQ
             stringstream message;
             //message<<pwrPlt[2]<<",";
             //message<<pwrPlt[3]<<",";
             message<< mean_power <<",";
             message<< mean_power <<",";
             message<<endl;
             cout << message <<endl;
             zmq::message_t zmq_message(message.str().length());
             memcpy((char *) zmq_message.data(), message.str().c_str(), message.str().length());
             publisher.send(zmq_message);
             cout<<"You passed the publisher"<<endl;
             // receive ball pos from python
             zmq::message_t ball_msg;
             subscriber.recv(&ball_msg);
             cout<<"You passed the subscriber"<<endl;
             // convert reveived data into c++ string/sstream
             string feat_str(((char *)ball_msg.data()));
             stringstream ss;
             ss.str(feat_str);
             cout<<"You passed the string"<<endl;
             float x=0;
             ss>>x;
             cout<<"Python sent: "<<x<<endl;



             //cb_power.put(mean_power);

            // update plot



            //vec power_vec;
            //cb_power.peek(power_vec);
            //mean_power = mean(power_vec);
            cout<<"mean power: "<<mean_power<<endl;
            if (mean_power>alpha_1){
                            y_position=(beta_0-beta_1)*mean_power/(alpha_0-alpha_1)+(alpha_0*beta_1-alpha_1*beta_0)/(alpha_0-alpha_1);
                           }
            alpha_sat=((225-beta_1)-(beta_2-beta_1)*alpha_1/(alpha_1-alpha_2))*((alpha_1-alpha_2)/(beta_1-beta_2));
            if (mean_power<=alpha_1)
            {
                y_position=y_position=mean_power*(beta_1-beta_2)/(alpha_1-alpha_2)+(beta_2*alpha_1-beta_1*alpha_2)/(alpha_1-alpha_2);;
            }
            //double alpha_sat=(100-(beta_2-beta_1)*alpha_2/(alpha_1-alpha_2))*((alpha_1-alpha_2)/(beta_1-beta_2));
            if (mean_power<alpha_sat){
                y_position=225;
            }
            cout<<"Y_position"<<y_position<<endl;
            redRect.y=y_position;
            //redRect.y=5000;
            //bounds for the boxes, if the box is horizontal, it should be redRect.x
            cout<<"Y position: "<<redRect.y<<endl;
            if (redRect.y > 620)
                redRect.y = 600;
            else if (redRect.y < 200)
                redRect.y = 200;

            //Given the current position and the original settings, returns the next target
            //target = stateMachine.UpdateState(redRect.y);
            cout<<"Rest counter is "<<rest_counter<<endl;
            cout<<"Active counter is "<<rest_counter<<endl;
            cout<<"Target is "<<target<<endl;
            if (mean_power>alpha_1){
                rest_counter+=1;
              }
            else{
                rest_counter=0;

            }
            if(target==0){//This is the rest state
                if(rest_counter>=3*60){
                    target=1;
                    reward_money+=0.25;
                }
            }
            else{ //This is the active state
                if (active_counter<5*60) {
                    target=1;
                    active_counter+=1;
                }
                else{
                target=0;//Change the cue to zero
                rest_counter=0;
                active_counter=0;
                }

            }
            if(target==1){
                if(redRect.y<425){
                    if(active_counter==2*60){
                        reward_money+=0.5;
                    }
                }
            }

            //cout<<"x: "<<redRect.x<<endl;
            cout<<"Out Target: "<<target<<endl;
            // SDL section
            //In charge of controlling the vertical boxes shape color and changes
            DrawGraphics_Augmented(screen,target,reward_money,&redRect);

            // write into csv
            //This routine pushes all the available information in a csv file named 'data.csv'
            //The Format is:
            //|Target|Y position of cursor|Left average power|right average power| channel1|.....|channel64|time|
            //The location is in the same directory as the application
           }
           clock_gettime(CLOCK_REALTIME, &requestEnd);
           double accum = ( requestEnd.tv_sec - requestStart.tv_sec )
             + ( requestEnd.tv_nsec - requestStart.tv_nsec )
             / 1E9;
           cout<<"t: "<<fixed<<setprecision(12)<<accum<<endl;
           loop = 0;
           gnuplot.Plot(pwrPlt);
         }
       }

     fclose(pFile);
     gui.join();
    SDL_ShowCursor(SDL_ENABLE);

    return 0;
}

