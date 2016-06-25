#ifndef FFTREALFLOAT_HPP
#define FFTREALFLOAT_HPP

#include <stdio.h>
#include <fftw3.h>
#include "FFTRealFloat_Mixed.hpp"
#include "fftengine.hpp"

struct FFTRealFloat : public FFTRealFloat_Mixed
{
private:
    int N;
    float* wavetable = NULL;
    inline int N_bins() { return N & 1 ? (N + 1) / 2 : N/2 + 1; }

public:

    FFTRealFloat(int n)
    {
        N = n;
        if ( !wavetable )
            wavetable = new float[((N * 2) + 15)];

        rffti(N, wavetable);
    }

    void ft(float* x)
    {
        rfftf(N, x, wavetable);
    }

    void ft(float* x, fftwf_complex* y)
    {
        rfftf(N, x, wavetable);

        y[0][0] = x[0];
        y[0][1] = 0.f;
        for (int i = 1; i < N_bins(); i++)
        {
            y[i][0] = x[(i * 2) - 1];
            y[i][1] = x[i * 2];
        }
        if ( !(N & 1) )
        {
            y[N / 2][0] = x[N - 1];
            y[N / 2][1] = 0.f;
        }
    }

    void bt(float* x)
    {
        rfftb(N, x, wavetable);
    }

    void bt(fftwf_complex* x, float* y)
    {
        y[0] = x[0][0];
        for (int i = 1; i < N_bins(); i++)
        {
            y[(i * 2) - 1] = x[i][0];
            y[i * 2] = x[i][0];
        }
        if ( !(N & 1) )
            y[N - 1] = x[N / 2][0];

        rfftb(N, y, wavetable);
    }
};

#endif
