#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include <stdlib.h>
#include <stdio.h>

#include <portaudio.h>

/* Select sample format. */
#if 1
    #define PA_SAMPLE_TYPE  paFloat32
    typedef float SAMPLE;
    #define SAMPLE_SILENCE  (0.0f)
    #define PRINTF_S_FORMAT "%.8f"
#elif 0
    #define PA_SAMPLE_TYPE  paInt16
    typedef short SAMPLE;
    #define SAMPLE_SILENCE  (0)
    #define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
    typedef char SAMPLE;
    #define SAMPLE_SILENCE  (0)
    #define PRINTF_S_FORMAT "%d"
#else
    #define PA_SAMPLE_TYPE  paUInt8
    typedef unsigned char SAMPLE;
    #define SAMPLE_SILENCE  (128)
    #define PRINTF_S_FORMAT "%d"
#endif

struct WpiEngine
{
    PaError             err = paNoError;
    ~WpiEngine()        { terminate(); }
    void init();
    void terminate();
    void checkPaError();

    int numDevices;
    void printDevs();
    void printSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);

    PaStreamParameters* inputParameters     = NULL;
    PaStreamParameters* outputParameters    = NULL;
    PaStream*           stream;
    FILE*               fid                 = NULL;
    const char*         filename            = NULL;

    unsigned int        NUM_SECONDS         = 10;
    unsigned int        NUM_CHANNELS        = 2;    // stereo input
    unsigned int        SAMPLE_RATE         = 44100;
    unsigned int        FRAMES_PER_BUFFER   = 2048;

    SAMPLE*             wavetable           = NULL;

    inline size_t numFrames()               { return NUM_SECONDS * SAMPLE_RATE; }
    inline size_t numSamples()              { return numFrames() * NUM_CHANNELS; }
    inline size_t numBytes()                { return numSamples() * sizeof(SAMPLE); }

    #define DITHER_FLAG     (paDitherOff)

    void initFFTData();
    void selectDefaultInputParameters();
    void selectDefaultOutputParameters();

    void record();
    int  recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

    void play();
    int  playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

    void genSineWavetable(double frequency);
    void genEmptyWavetable();

    void playWave();
    int  playWaveCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

    SAMPLE*     fftData     = NULL;
    int         fftBins     = 32768;
};

#endif
