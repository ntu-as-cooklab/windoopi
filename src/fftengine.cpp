#include "fftengine.hpp"

#include <math.h>
#include <stdio.h>

#include "FFTRealFloat.hpp"

void FFTEngine::init()
{
    fftin   = (float*)          fftwf_malloc(sizeof(float) * N);
    fftout  = (fftwf_complex*)  fftwf_malloc(sizeof(fftwf_complex) * N);
    fftplan = fftwf_plan_dft_r2c_1d(N, fftin, fftout, FFTW_ESTIMATE);
}

FFTEngine::~FFTEngine()
{
    fftwf_destroy_plan(fftplan);
    fftwf_free(fftin);
    fftwf_free(fftout);
}

void FFTEngine::fft()
{
    static FFTRealFloat fftRealFloat(N);

    fftRealFloat.ft(fftin, fftout);

    //fftwf_execute(fftplan);
}

void FFTEngine::hanning()
{
    for (int i=0; i<N; i++)
        fftin[i] *= 0.5f * ( 1 - cos( 2*M_PI*i/(N-1) ) );
}
