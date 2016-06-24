#ifndef FFTENGINE_HPP
#define FFTENGINE_HPP

#include <fftw3.h>

struct FFTEngine
{
    int N = 2048;
    int fftBins = 32768;
    fftwf_plan plan;

    float*          in;
    fftwf_complex*  out;

    void execute();
    void init();
    ~FFTEngine();
};

#endif
