#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include "paengine.hpp"
#include "fftengine.hpp"

//#define DITHER_FLAG   (paDitherOff)
typedef short SAMPLE;

struct WpiEngine : public PaEngine, public FFTEngine
{
    float       OUTPUT_FREQUENCY;
    SAMPLE*     wavetable           = NULL;

    WpiEngine()
    {
        PA_SAMPLE_TYPE      = paInt16;
        NUM_CHANNELS        = 1;        // mono
        SAMPLE_RATE         = 44100;
        FRAMES_PER_BUFFER   = 1024;
        N                   = 8192;
        genWavetable (1.5e3);
    }
    ~WpiEngine()
    {
        //delete [] wavetable;
        //wavetable = NULL;
    }

    float F_max()                       { return SAMPLE_RATE / 2.f; }
    float resolution()                  { return SAMPLE_RATE / (float) N; }
    float Lowest_Detectable_Frequency() { return 5 * resolution(); }

    void init();
    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    void getFrequency();

    void genWavetable(double frequency);
    void genSineWavetable(double frequency);
    void genEmptyWavetable();
};

#endif
