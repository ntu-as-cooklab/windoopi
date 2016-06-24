#ifndef PAENGINE_HPP
#define PAENGINE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

/* Select sample format. */
#if 1
    #define PA_SAMPLE_TYPE  paFloat32
    typedef float SAMPLE;
    #define PRINTF_S_FORMAT "%.8f"
#elif 0
    #define PA_SAMPLE_TYPE  paInt16
    typedef short SAMPLE;
    #define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
    typedef char SAMPLE;
    #define PRINTF_S_FORMAT "%d"
#else
    #define PA_SAMPLE_TYPE  paUInt8
    typedef unsigned char SAMPLE;
    #define SAMPLE_SILENCE  (128)
    #define PRINTF_S_FORMAT "%d"
#endif

struct PaEngine
{
    PaError             err = paNoError;
    void init();
    void terminate();
    void checkPaError();
    ~PaEngine()         { terminate(); }

    int numDevices;
    void printDevs();
    void printSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);

    unsigned int        NUM_CHANNELS        = 2;    // stereo input
    unsigned int        SAMPLE_RATE         = 44100;
    unsigned int        FRAMES_PER_BUFFER   = 2048;

    PaStreamParameters* inputParameters     = NULL;
    PaStreamParameters* outputParameters    = NULL;
    PaStream*           stream;
    FILE*               fid                 = NULL;
    const char*         filename            = NULL;
    SAMPLE*             wavetable           = NULL;

    void selectDefaultInputParameters();
    void selectDefaultOutputParameters();

    void genSineWavetable(double frequency = 1.5e4);
    void genEmptyWavetable();

    #define DITHER_FLAG     (paDitherOff)
};

#endif
