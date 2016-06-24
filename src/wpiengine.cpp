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

static int windooCallbackWrapper( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    return ((WpiEngine*) userData)->windooCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int WpiEngine::windooCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags)
{
    const   SAMPLE          *rptr           = (const SAMPLE*) inputBuffer;
            SAMPLE          *wptr           = & sampleData[frameIndex * NUM_CHANNELS];
            size_t          framesToCalc;
            int             finished;

    // Prevent unused variable warnings
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    if( size_t framesLeft = numFrames() - frameIndex < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    static float cumulatedVolume = 0;
    static size_t cumulatedFrames = 0;

    for(size_t i = 0; i < framesToCalc; i++ )
        for (unsigned int n = 0; n < NUM_CHANNELS; n++)
            cumulatedVolume += (*wptr++ = inputBuffer ? *rptr++ : SAMPLE_SILENCE);
    frameIndex += framesToCalc;

    // Calculate and display current volume
    cumulatedFrames += framesToCalc;
    if (cumulatedFrames >= SAMPLE_RATE/100)
    {
        cumulatedVolume /= cumulatedFrames;
        //printf ("%f\n", cumulatedVolume);
        for (int i = 0; i<75; i++) printf (" ");
        printf ("\r");
        for (int i = 0; i<cumulatedVolume*250; i++) printf ("|");
        printf ("\r");
        fflush(stdout);
        cumulatedVolume = 0;
        cumulatedFrames = 0;
    }

    return finished;
}
