#ifndef PAENGINE_HPP
#define PAENGINE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

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

    unsigned int        NUM_CHANNELS        = 2;    // stereo
    unsigned int        SAMPLE_RATE         = 44100;
    unsigned int        FRAMES_PER_BUFFER   = 2048;
    PaSampleFormat      PA_SAMPLE_TYPE      = paFloat32;

    PaStreamParameters* inputParameters     = NULL;
    PaStreamParameters* outputParameters    = NULL;
    PaStream*           stream;
    FILE*               fid                 = NULL;
    const char*         filename            = NULL;

    void selectDefaultInputParameters();
    void selectDefaultOutputParameters();
};

#endif
