#include "fftengine.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "FFTRealDouble.hpp"

void FFTEngine::init()
{
    fftin   = new short[N];
    fftdata = new double[N];
    fftout  = new fftw_complex[N];
    fftplan = fftw_plan_dft_r2c_1d(N, fftdata, fftout, FFTW_ESTIMATE);
}

FFTEngine::~FFTEngine()
{
    fftw_destroy_plan(fftplan);

    delete [] fftin;
    delete [] fftdata;
    delete [] fftout;
}

void FFTEngine::fft()
{
    static FFTRealDouble fftRealDouble(N);
    fftRealDouble.ft(fftdata, fftout);

    //fftwf_execute(fftplan);
}

void FFTEngine::hanning()
{
    for (int i=0; i<N; i++)
        fftdata[i] = 0.5f * ( 1 - cos( 2*M_PI*i/(N-1) ) ) * fftin[i];
}
