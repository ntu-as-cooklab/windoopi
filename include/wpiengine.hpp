#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include "paengine.hpp"

struct WpiEngine : public PaEngine
{
    ~WpiEngine();

    void initFFTData();

    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

    SAMPLE*     fftData     = NULL;
    int         fftBins     = 32768;
};

#endif
