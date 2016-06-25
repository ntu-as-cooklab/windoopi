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
        FRAMES_PER_BUFFER   = 512;
        N                   = 32768;
        genSineWavetable (1.5e4);
    }
    float F_max()  { return SAMPLE_RATE / 2.f; }
    float resolution()  { return SAMPLE_RATE / N; }
    float Lowest_Detectable_Frequency() { return 5 * SAMPLE_RATE / N; }
    float Window_Size() { return 5 * SAMPLE_RATE / 100.f; }

    void init();
    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    void getFrequency();
};

#endif
