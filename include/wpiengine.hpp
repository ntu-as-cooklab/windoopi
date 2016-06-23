#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include <stdlib.h>
#include <stdio.h>

#include <portaudio.h>

struct WpiEngine
{
    PaError err;

    int numDevices;

    ~WpiEngine()        { terminate(); }

    void init();
    void terminate();
    void checkPaError();

    void printDevs();
    void printSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
};

#endif
