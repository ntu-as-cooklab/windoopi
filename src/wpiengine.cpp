#include "wpiengine.hpp"

#include <string.h>

void WpiEngine::init()
{
    err = Pa_Initialize();
    printf( "\n%s\nversion number %d\n\n", Pa_GetVersionText(), Pa_GetVersion());
    checkPaError();
}

void WpiEngine::terminate()
{
    err = Pa_Terminate();
}

void WpiEngine::checkPaError()
{
    if(err != paNoError)
    {
        printf("PortAudio error %d: %s\n", err, Pa_GetErrorText(err));
        terminate();
        exit(err);
    }
}

void WpiEngine::initSampleData()
{
    frameIndex = 0;
    delete [] sampleData;
    if ( ! (sampleData = new SAMPLE[numSamples()]) )  return printf("Could not allocate record array.\n"), terminate();
    memset (sampleData, 0.f, numSamples() * sizeof(SAMPLE));
}
