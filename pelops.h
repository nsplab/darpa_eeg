#ifndef PELOPS_H
#define PELOPS_H

#include <libusb-1.0/libusb.h>

namespace arma{
template<typename eT> class Col;
typedef Col <double> vec;
}

class LinAmp {
 public:
    static unsigned char buffer_sginal_byte[260];
    static unsigned char buffer[65536];
    static const size_t vendorID = 0x153c;
    static const size_t productID = 0x0002;
    static unsigned char init_seq1[];
    static unsigned char init_seq2[];

    libusb_context *ctx;
    libusb_device_handle *dev_handle;
    static libusb_transfer* trans;

    static void (*filter_step)(arma::vec samples);

    LinAmp(void (*filter_step_func)(arma::vec sample) = NULL);
    ~LinAmp();


    bool Init();
    void StartSampling();
    void StopSampling();
    static void LIBUSB_CALL call_back(libusb_transfer* transfer);
    void ReadImpedance();
};

#endif // PELOPS_H
