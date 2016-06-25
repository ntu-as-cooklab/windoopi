#include "paengine.hpp"

#include <math.h>
#include <string.h>

void PaEngine::init()
{
    err = Pa_Initialize();
    printf( "\n%s\nversion number %d\n\n", Pa_GetVersionText(), Pa_GetVersion());
    checkPaError();
}

void PaEngine::terminate()
{
    err = Pa_Terminate();
    delete inputParameters;
    delete outputParameters;
    delete [] wavetable;
    inputParameters     = NULL;
    outputParameters    = NULL;
    wavetable           = NULL;
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
    if ( (inputParameters->device = Pa_GetDefaultInputDevice()) == paNoDevice )  return printf("Error: No default input device.\n"), terminate();
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
    if ( (outputParameters->device = Pa_GetDefaultOutputDevice()) == paNoDevice )  return printf("Error: No default output device.\n"), terminate();
    outputParameters->channelCount = NUM_CHANNELS;
    outputParameters->sampleFormat = PA_SAMPLE_TYPE;
    outputParameters->suggestedLatency = Pa_GetDeviceInfo( outputParameters->device )->defaultLowOutputLatency;
    outputParameters->hostApiSpecificStreamInfo = NULL;
}

void PaEngine::genSineWavetable(double frequency)
{
    // initialise sinusoidal wavetable
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[SAMPLE_RATE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    for( size_t i = 0; i < SAMPLE_RATE; i++ )
        wavetable[i] =  (float) sin( 2.f * M_PI * ((double)i) / ((double)SAMPLE_RATE) * frequency );
}

void PaEngine::genEmptyWavetable()
{
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[SAMPLE_RATE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    memset (wavetable, 0.f, SAMPLE_RATE * sizeof(SAMPLE));
}
