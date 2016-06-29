#ifndef FFTENGINE_HPP
#define FFTENGINE_HPP

#include <fftw3.h>

struct FFTEngine
{
    int N;
    inline int N_bins() { return N & 1 ? (N + 1) / 2 : N/2 + 1; }

    fftw_plan      fftplan;
    short*         fftin;
    double*        fftdata;
    fftw_complex*  fftout;

    void hanning();
    void fft();
    void init();
    void getFrequency();
    ~FFTEngine();
};

#endif
