#include "fftengine.hpp"

void FFTEngine::init()
{
    in      = (float*)          fftwf_malloc(sizeof(float) * N);
    out     = (fftwf_complex*)  fftwf_malloc(sizeof(fftwf_complex) * N);
    plan    = fftwf_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
}

FFTEngine::~FFTEngine()
{
    fftwf_destroy_plan(plan);
    fftwf_free(in);
    fftwf_free(out);
}

void FFTEngine::execute()
{
    fftwf_execute(plan);
}
