#include "fftengine.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "FFTRealFloat.hpp"

void FFTEngine::init()
{
    fftin   = (short*)          malloc(sizeof(short) * N);
    fftdata = (float*)          malloc(sizeof(float) * N);
    fftout  = (fftwf_complex*)  malloc(sizeof(fftwf_complex) * N);
    fftplan = fftwf_plan_dft_r2c_1d(N, fftdata, fftout, FFTW_ESTIMATE);
}

FFTEngine::~FFTEngine()
{
    fftwf_destroy_plan(fftplan);

    //free(fftin);
    //free(fftout);
}

void FFTEngine::fft()
{
    static FFTRealFloat fftRealFloat(N);

    fftRealFloat.ft(fftdata, fftout);

    //fftwf_execute(fftplan);
}

void FFTEngine::hanning()
{
    for (int i=0; i<N; i++)
        fftdata[i] = 0.5f * ( 1 - cos( 2*M_PI*i/(N-1) ) ) * fftin[i];
}
