#include "wpiengine.hpp"

#include <string.h>
#include <math.h>

void WpiEngine::init()
{
    err = Pa_Initialize();
    printf( "\n%s\nversion number %d\n\n", Pa_GetVersionText(), Pa_GetVersion());
    checkPaError();
}

void WpiEngine::terminate()
{
    err = Pa_Terminate();
    if (inputParameters)    delete inputParameters;
    if (outputParameters)   delete outputParameters;
    if (sampleData)         delete [] sampleData;
    if (wavetable)          delete [] wavetable;
    inputParameters     = NULL;
    outputParameters    = NULL;
    sampleData          = NULL;
    wavetable           = NULL;
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
    sampleData = NULL;
    if ( ! (sampleData = new SAMPLE[numSamples()]) )  return printf("Could not allocate sample data array.\n"), terminate();
    memset (sampleData, 0.f, numSamples() * sizeof(SAMPLE));
}

void WpiEngine::initSineWavetable()
{
    // initialise sinusoidal wavetable
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[TABLE_SIZE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    for( size_t i = 0; i < TABLE_SIZE; i++ )
        wavetable[i] =  (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2.f );
}
