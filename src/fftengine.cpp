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

inline float magnitude(fftwf_complex z)
{
    return sqrt(z[0]*z[0] + z[1]*z[1]);
}

void FFTEngine::getFrequency()
{
    float resolution = 44100.f / N;

    int bin = 0;
    float max = 0.f;
    for (int i = 0; i < N_bins(); i++)
        if (magnitude(fftout[i]) > max && resolution * i < 1.2e4)
        {
            bin = i;
            max = magnitude(fftout[i]);
        }
    float freq = resolution * bin;
    printf ("Bin: %4d,\tFreq: %12.6f,\tMax: %12.6f\n", bin, freq, max);
}
