#include "pelops.h"
#include "seqs.h"

#include <iostream>
#include <iomanip>
#include <armadillo>

using namespace std;
using namespace arma;

unsigned char LinAmp::buffer_sginal_byte[0x104] = {0};
unsigned char LinAmp::buffer[65536] = {0};
libusb_transfer* LinAmp::trans = NULL;
void (*LinAmp::filter_step)(vec samples) = NULL;

LinAmp::LinAmp(void (*filter_step_func)(vec)) {
    filter_step = filter_step_func;
    ctx = NULL;
}

//**************************************************
// Release the claimed interface and close the connection
//**************************************************
LinAmp::~LinAmp() {
    if (dev_handle != NULL) {
        libusb_release_interface(dev_handle, 0);
        libusb_close(dev_handle);
    }
    libusb_exit(ctx);
}

//**************************************************
// open the device and claim the default interface
//**************************************************
bool LinAmp::Init() {
    int r = libusb_init(&ctx);
    if(r < 0) {
        return false;
    }
    libusb_set_debug(ctx, 3); //verbosity level 3,

    dev_handle = libusb_open_device_with_vid_pid(ctx, vendorID, productID);
    if(dev_handle == NULL) {
        libusb_exit(ctx);
        return false;
    }

    r = libusb_claim_interface(dev_handle, 0);
    if(r < 0) {
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return false;
    }

    int transferred = 0;

    // initialize the device
    libusb_control_transfer(dev_handle, 0x40, 0xD2, 0, 0, NULL, 0, 1000);
    libusb_interrupt_transfer(dev_handle, 0x81, buffer, 65536, &transferred, 1000);

    //cout<<showbase<<internal<<setfill('0');
    //cout<<"data: "<<hex<<(int)buffer[0]<<"  "<<(int)buffer[1]<<endl;

    // read configuration
    libusb_control_transfer(dev_handle, 0x40, 0xDC, 0, 0, NULL, 0, 1000);
    libusb_interrupt_transfer(dev_handle, 0x81, buffer, 65536, &transferred, 1000);
    //cout<<"conf: "<<hex<<(int)buffer[0]<<"  "<<(int)buffer[1]<<"  "<<(int)buffer[2]<<endl;

    // write configuration
    libusb_control_transfer(dev_handle, 0xC0, 0xB7, 0, 0, buffer, 0x100, 1000);
    libusb_control_transfer(dev_handle, 0xC0, 0xD8, 0, 0, buffer, 0x02, 1000);

    libusb_control_transfer(dev_handle, 0x40, 0xD9, 0, 0, NULL, 0, 1000);
    libusb_bulk_transfer(dev_handle, 0x01, init_seq1, 0x13ec, &transferred, 500);

    libusb_control_transfer(dev_handle, 0x40, 0xDB, 0, 0, NULL, 0, 1000);
    libusb_bulk_transfer(dev_handle, 0x01, init_seq2, 0x142b, &transferred, 500);

    // read configuration and confirm wrote successfully
    libusb_control_transfer(dev_handle, 0x40, 0xDC, 0, 0, NULL, 0, 1000);
    libusb_interrupt_transfer(dev_handle, 0x81, buffer, 65536, &transferred, 1000);
    //cout<<"conf: "<<hex<<(int)buffer[0]<<"  "<<(int)buffer[1]<<"  "<<(int)buffer[2]<<endl;
    if (buffer[0] != init_seq2[0] || buffer[1] != init_seq2[1])
        return false;

    return true;
}

void LinAmp::StartSampling() {
    // Request the stream
    libusb_control_transfer(dev_handle, 0x40, 0xF7, 0, 0, NULL, 0, 1000);

    // asynchronous I/O
    trans = libusb_alloc_transfer(0);
    libusb_fill_interrupt_transfer(trans, dev_handle, 0x81,  buffer_sginal_byte, 0x104, libusb_transfer_cb_fn(&call_back), NULL, 1000);
    libusb_submit_transfer(trans);
}

void LinAmp::StopSampling() {
    libusb_control_transfer(dev_handle, 0x40, 0xB8, 0, 0, NULL, 0, 1000);
    libusb_free_transfer(trans);
}

void LIBUSB_CALL LinAmp::call_back(libusb_transfer* transfer) {
    if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        vec samples;
        float sample = 0.0f;
        for (size_t i=0; i<64; i++) {
            memcpy(&(buffer_sginal_byte[i]), &sample, sizeof(float));
            samples<<sample;
        }

        filter_step(samples);

        libusb_submit_transfer(trans);
    }
}

void LinAmp::ReadImpedance() {
    //libusb_control_transfer(dev_handle, 0x40, 0xD9, 0, 0, NULL, 0, 1000);
}


// test
void test_func(vec samples) {
    cout<<samples<<endl;
}

int main() {
    LinAmp lamp(test_func);
    if (lamp.Init() == false) {
        cout<<"Failed to initialize the device"<<endl;
        return 1;
    }

    lamp.StartSampling();
    usleep(10000);
    lamp.StopSampling();
}
