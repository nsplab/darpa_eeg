#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <thread>
#include <fstream>

#include <itpp/signal/source.h>
#include <itpp/signal/sigfun.h>
#include <itpp/base/circular_buffer.h>
#include <itpp/stat/misc_stat.h>
#include <itpp/base/random.h>
#include <itpp/base/math/min_max.h>
#include <itpp/base/vec.h>

#include <SDL/SDL.h>

#include "utils.h"
#include "gnuplot.h"
#include "eeg_receiver.h"
#include "statemachine.h"

using namespace std;
using namespace itpp;


int main()
{
    // alpha value in first order autoregressive
    float alpha = 0.9995;

    // signals gui window to close
    bool exit = false;

    // the ball/cursor whose vertical position(redRect.y) is controlled by EEG
    SDL_Rect redRect;
    redRect.x = 960;
    redRect.y = 500;
    redRect.h = 50; // height
    redRect.w = 50; // width


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
    EegReceiver eeg;

    // log files
    ofstream csv("/media/ssd/uruk/data.csv");
    //ofstream r_csv("/media/ssd/uruk/right_power_data.csv");

    // gnuplot window to plot live data
    //GnuPlot gnuplot;


    float channels[65];
    string str;
    size_t counter = 0;

    double left_av_mu_power = 0.0;
    double right_av_mu_power = 0.0;

    // circular buffers to hold filtered eeg values, last 66 values
    Circular_Buffer<double> cb_eeg_left(66);
    Circular_Buffer<double> cb_eeg_right(66);
    // initialize circular buffers
    for (size_t i=0; i<66; i++) {
        cb_eeg_left.put(0.0);
        cb_eeg_right.put(0.0);
    }

    // circular buffer to hold filtered power, last 200 values
    Circular_Buffer<double> cb_power(10);


    // target position, -1(down), 0 ,1(up)
    int target = 0;

    // random length of wait rest and exercise periods
    size_t wait_time = rand() % 10 + 10;
    size_t go_time = rand() % 3 + 10;

    timeval t1, t2;
    double elapsedTime;
    gettimeofday(&t1, NULL);

    //*********************************************************************
    //*********************************************************************
    //** BASELINE
    //*********************************************************************
    //*********************************************************************

    // record baseline and compute bounds
    // relax time
    sleep(5);
    // baseline time
    ofstream relax_csv("/media/ssd/uruk/new.csv");
    ofstream left_relax_csv("/media/ssd/uruk/new_left.csv");
    ofstream right_relax_csv("/media/ssd/uruk/new_right.csv");
    Vec<double> baselineSamples;
    Vec<double> baselineSamples_left;
    Vec<double> baselineSamples_right;
    bool first_iteration = true;
    while(true){
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec);

        // TODO: set parameter
        if (elapsedTime > 20.0){
            break;
        }

        eeg.receive(channels);

        // large laplacian filter of C3
        cb_eeg_left.put(channels[27] - 0.25 * (channels[25]+channels[9]+channels[29]+channels[45]) );
        // large laplacian filter of C4
        cb_eeg_right.put(channels[31] - 0.25 * (channels[29]+channels[13]+channels[33]+channels[49]) );

        // every 13 cycles ~ 50 ms (= 13/256Hz) compute spectrum
        counter += 1;
        if (counter == 13) {
            counter = 0;

            // get values circular buffers in vectors
            Vec<double> left_signal;
            Vec<double> right_signal;
            cb_eeg_left.peek(left_signal);
            cb_eeg_right.peek(right_signal);

            // compute spectrum of signals, 256 Hz sampling rate
            vec left_spd = spectrum(left_signal, 256.0);
            vec right_spd = spectrum(right_signal, 256.0);

            //cout<<"left_spd: "<<left_spd<<endl;

            // compute mean power in mu-alpha band
            double left_mu_power = 0.0f;
            double right_mu_power = 0.0f;
            for (size_t i=8; i<=12; i++) {
                left_mu_power += left_spd(i)/5.0;
                right_mu_power += right_spd(i)/5.0;
            }
            left_av_mu_power = (alpha) * left_av_mu_power + (1.0 - alpha) * left_mu_power;
            right_av_mu_power = (alpha) * right_av_mu_power + (1.0 - alpha) * right_mu_power;

            if (first_iteration) {
                left_av_mu_power = left_mu_power;
                right_av_mu_power = right_mu_power;
                first_iteration = false;
            }

            double mean_power = right_av_mu_power - left_av_mu_power;

            //cout<<"mean_power: "<<mean_power<<endl;

            baselineSamples = concat(baselineSamples,mean_power);
            baselineSamples_left = concat(baselineSamples_left,left_av_mu_power);
            baselineSamples_right = concat(baselineSamples_right,right_av_mu_power);
        }
    }

    //relax_csv<<baselineSamples<<endl;
    //left_relax_csv<<baselineSamples_left<<endl;
    //right_relax_csv<<baselineSamples_right<<endl;

    float mean_baseline = mean(baselineSamples);

    //float max_baseline =  2.0*variance(baselineSamples) + mean_baseline;
    //float min_baseline = -2.0*variance(baselineSamples) + mean_baseline;

    float max_baseline =  max(baselineSamples);
    float min_baseline =  min(baselineSamples);


    cout<<"mean_baseline: "<<mean_baseline<<endl;
    cout<<"max_baseline: "<<max_baseline<<endl;
    cout<<"min_baseline: "<<min_baseline<<endl;

    StateMachine stateMachine(0.075, 15.0, 1.0, 1275, 645);

    //*********************************************************************
    //*********************************************************************
    //** Experiment
    //*********************************************************************
    //*********************************************************************
    // main loop

    timeval init_time;
    gettimeofday(&init_time, NULL);

     counter = 0;
     while (!exit) {

        // receive EEG
        eeg.receive(channels);

        // large laplacian filter of C3
        cb_eeg_left.put(channels[27] - 0.25 * (channels[25]+channels[9]+channels[29]+channels[45]) );
        // large laplacian filter of C4
        cb_eeg_right.put(channels[31] - 0.25 * (channels[29]+channels[13]+channels[33]+channels[49]) );

        // every 13 cycles ~ 50 ms (= 13/256Hz) compute spectrum
        counter += 1;
        if (counter == 13) {
            counter = 0;

            // get values circular buffers in vectors
            Vec<double> left_signal;
            Vec<double> right_signal;
            cb_eeg_left.peek(left_signal);
            cb_eeg_right.peek(right_signal);

            // compute spectrum of signals, 256 Hz sampling rate
            vec left_spd = spectrum(left_signal, 256.0);
            vec right_spd = spectrum(right_signal, 256.0);

            // compute mean power in mu-alpha band
            float left_mu_power = 0.0f;
            float right_mu_power = 0.0f;
            for (size_t i=8; i<=12; i++) {
                left_mu_power += left_spd(i)/5.0;
                right_mu_power += right_spd(i)/5.0;
            }
            left_av_mu_power = (alpha) * left_av_mu_power + (1.0 - alpha) * left_mu_power;
            right_av_mu_power = (alpha) * right_av_mu_power + (1.0 - alpha) * right_mu_power;

            float mean_power = right_av_mu_power - left_av_mu_power;
            //cb_power.put(mean_power);

            // update plot
            //gnuplot.Plot(right_av_mu_power);


            //vec power_vec;
            //cb_power.peek(power_vec);
            //mean_power = mean(power_vec);
            cout<<"mean power: "<<mean_power<<endl;
            cout<<"mean base: "<<mean_baseline<<endl;
            cout<<"min base: "<<min_baseline<<endl;
            cout<<"max base: "<<max_baseline<<endl;

            if (mean_power < mean_baseline)
                mean_power = (mean_power - mean_baseline) / (min_baseline - mean_baseline) * -115.0; // -315
            else
                mean_power = (mean_power - mean_baseline) / (max_baseline - mean_baseline) * 115.0; // 315

            cout<<"mean power: "<<mean_power<<endl;

            redRect.x = 960.0 + mean_power;
            if (redRect.x > 1870)
                redRect.x = 1870;
            else if (redRect.x < 50)
                redRect.x = 50;

            int target = stateMachine.UpdateState(redRect.x);
            //cout<<"x: "<<redRect.x<<endl;

            // SDL section
            DrawGraphics(screen, target, &redRect);

            // write into csv
            timeval now;
            long int elapsedTime;
            gettimeofday(&now, NULL);
            elapsedTime = (now.tv_sec * 1000000 + now.tv_usec) -  (init_time.tv_sec *1000000 - init_time.tv_usec);
            double dt = elapsedTime / 1000000.0;
            csv<<target<<","<<redRect.x<<","<<left_av_mu_power<<","<<right_av_mu_power<<",";
            // 64 channel + digital in
            for (int i=0; i<65; i++){
                csv<<channels[i]<<",";
            }
            csv<<dt<<endl;

            csv.flush();

        }
    }

    gui.join();
    SDL_ShowCursor(SDL_ENABLE);

    return 0;
}

