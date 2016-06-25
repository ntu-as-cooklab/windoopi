#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include "paengine.hpp"
#include "fftengine.hpp"

struct WpiEngine : public PaEngine, public FFTEngine
{
    WpiEngine()
    {
        NUM_CHANNELS        = 1;        // mono
        SAMPLE_RATE         = 44100;
        FRAMES_PER_BUFFER   = 2048;
        N                   = 2048;
        genSineWavetable (1.5e4);
    }
    float resolution()  { return SAMPLE_RATE / N; }

    void init();
    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    void getFrequency();
};

#endif
