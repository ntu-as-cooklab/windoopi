#ifndef FFTENGINE_HPP
#define FFTENGINE_HPP

#include <fftw3.h>

struct FFTEngine
{
    int N;
    inline int N_bins() { return N & 1 ? (N + 1) / 2 : N/2 + 1; }

    fftwf_plan      fftplan;
    float*          fftin;
    fftwf_complex*  fftout;

    void hanning();
    void fft();
    void init();
    void getFrequency();
    ~FFTEngine();
};

#endif
