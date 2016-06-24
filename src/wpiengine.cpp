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
    if (fftData)            delete [] fftData;
    if (wavetable)          delete [] wavetable;
    inputParameters     = NULL;
    outputParameters    = NULL;
    fftData             = NULL;
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

void WpiEngine::initFFTData()
{
    delete [] fftData;
    fftData = NULL;
    if ( ! (fftData = new SAMPLE[numSamples()]) )  return printf("Could not allocate FFT data array.\n"), terminate();
    memset (fftData, 0.f, numSamples() * sizeof(SAMPLE));
}

void WpiEngine::selectDefaultInputParameters()
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

void WpiEngine::selectDefaultOutputParameters()
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

void WpiEngine::genSineWavetable(double frequency = 1.5e4)
{
    // initialise sinusoidal wavetable
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[SAMPLE_RATE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    for( size_t i = 0; i < SAMPLE_RATE; i++ )
        wavetable[i] =  (float) sin( 2.f * M_PI * ((double)i) / ((double)SAMPLE_RATE) * frequency );
}

void WpiEngine::genEmptyWavetable()
{
    // initialise sinusoidal wavetable
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[SAMPLE_RATE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    memset (wavetable, 0.f, SAMPLE_RATE * sizeof(SAMPLE));
}
