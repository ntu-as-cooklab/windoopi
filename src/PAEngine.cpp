#include "paengine.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#include <cmath>
#include <string.h>

void PaEngine::init()
{
    err = Pa_Initialize();
    printf( "%s version number %d\n\n", Pa_GetVersionText(), Pa_GetVersion());
    checkPaError();
}

void PaEngine::terminate()
{
    err = Pa_Terminate();
    delete inputParameters;
    delete outputParameters;
    inputParameters     = NULL;
    outputParameters    = NULL;
}

void PaEngine::checkPaError()
{
    if(err != paNoError)
    {
        printf("PortAudio error %d: %s\n", err, Pa_GetErrorText(err));
        terminate();
        exit(err);
    }
}

void PaEngine::selectDefaultInputParameters()
{
    // Set input parameters
    delete inputParameters;
    inputParameters = new PaStreamParameters;
    if ( (inputParameters->device = Pa_GetDefaultInputDevice() ) == paNoDevice )  return printf("Error: No default input device.\n"), terminate();
    inputParameters->channelCount = NUM_CHANNELS;
    inputParameters->sampleFormat = PA_SAMPLE_TYPE;
    inputParameters->suggestedLatency = Pa_GetDeviceInfo( inputParameters->device )->defaultLowInputLatency;
    inputParameters->hostApiSpecificStreamInfo = NULL;
}

void PaEngine::selectDefaultOutputParameters()
{
    // Set output parameters
    delete outputParameters;
    outputParameters = new PaStreamParameters;
    if ( (outputParameters->device = Pa_GetDefaultOutputDevice() ) == paNoDevice )  return printf("Error: No default output device.\n"), terminate();
    outputParameters->channelCount = NUM_CHANNELS;
    outputParameters->sampleFormat = PA_SAMPLE_TYPE;
    outputParameters->suggestedLatency = Pa_GetDeviceInfo( outputParameters->device )->defaultLowOutputLatency;
    outputParameters->hostApiSpecificStreamInfo = NULL;
}
