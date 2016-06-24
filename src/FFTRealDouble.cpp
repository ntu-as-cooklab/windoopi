
#include <stdio.h>
#include "FFTRealDouble_Mixed.hpp"

struct FFTComplex1D
{
    double* x;
    double* y;
    int length;
};

struct FFTRealDouble : public FFTRealDouble_Mixed
{
private:
    int ndim;
    double* wavetable;

public:
    double norm_factor;

    FFTRealDouble(int fftbins)
    {
        ndim = fftbins;
        norm_factor = (double) fftbins;
        if ( !wavetable )
            wavetable = new double[((ndim * 2) + 15)];
        rffti(ndim, wavetable);
    }

    void ft(double* x, int x_length)
    {
        if (x_length != ndim)
        {
            printf("The length of data can not match that of the wavetable");
            return;
        }
        rfftf(ndim, x, wavetable);
    }

    void ft(double* x, FFTComplex1D y, int x_length)
    {
        if (x_length != ndim)
        {
            printf("The length of data can not match that of the wavetable");
            return;
        }

        rfftf(ndim, x, wavetable);
        if (ndim % 2 == 0)
            y.length = (ndim / 2) + 1;
        else
            y.length = (ndim + 1) / 2;
        y.x = new double[y.length];
        y.y = new double[y.length];

        y.x[0] = x[0];
        y.y[0] = 0.0;
        for (int i = 1; i < (ndim + 1) / 2; i++)
        {
            y.x[i] = x[(i * 2) - 1];
            y.y[i] = x[i * 2];
        }
        if (ndim % 2 == 0)
        {
            y.x[ndim / 2] = x[ndim - 1];
            y.y[ndim / 2] = 0.0;
        }
    }

    void bt(double* x, int x_length)
    {
        if (x_length != ndim)
        {
            printf("The length of data can not match that of the wavetable");
            return;
        }

        rfftb(ndim, x, wavetable);
    }

    void bt(FFTComplex1D x, double* y)
    {
        if (ndim % 2 == 0)
        {
            if (x.length != (ndim / 2) + 1)
            {
                printf("The length of data can not match that of the wavetable");
                return;
            }
        }
        else if (x.length != (ndim + 1) / 2)
        {
            printf("The length of data can not match that of the wavetable");
            return;
        }

        y[0] = x.x[0];
        for (int i = 1; i < (ndim + 1) / 2; i++)
        {
            y[(i * 2) - 1] = x.x[i];
            y[i * 2] = x.y[i];
        }
        if (ndim % 2 == 0)
            y[ndim - 1] = x.x[ndim / 2];

        rfftb(ndim, y, wavetable);
    }
};
