#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "wpiengine.hpp"

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallbackWrapper( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    return ((WpiEngine*) userData)->playCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int WpiEngine::playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags)
{
    const SAMPLE*   rptr                = & sampleData[sampleIndex()];
    SAMPLE*         wptr                = (SAMPLE*) outputBuffer;
    size_t          framesToProcess;
    int             finished;

    // Prevent unused variable warnings.
    (void) inputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    if( size_t framesLeft = numFrames() - frameIndex < framesPerBuffer )
    {
        // final buffer...
        framesToProcess = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToProcess = framesPerBuffer;
        finished = paContinue;
    }
    for( size_t i = 0; i < framesPerBuffer; i++ )
        for (unsigned int n = 0; n < NUM_CHANNELS; n++) *wptr++ = i < framesToProcess ? *rptr++ : 0;
    frameIndex += framesToProcess;

    return finished;
}

void WpiEngine::play()
{
    // Set output parameters
    delete outputParameters;
    outputParameters = new PaStreamParameters;
    if ( (outputParameters->device = Pa_GetDefaultOutputDevice()) == paNoDevice )  return printf("Error: No default output device.\n"), terminate();
    outputParameters->channelCount = NUM_CHANNELS;
    outputParameters->sampleFormat = PA_SAMPLE_TYPE;
    outputParameters->suggestedLatency = Pa_GetDeviceInfo( outputParameters->device )->defaultLowOutputLatency;
    outputParameters->hostApiSpecificStreamInfo = NULL;

    printf("\n=== Now playing back. ===\n"); fflush(stdout);
    frameIndex = 0;
    err = Pa_OpenStream(
              &stream,
              NULL, // no input
              outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      // we won't output out of range samples so don't bother clipping them
              playCallbackWrapper,
              this );
    checkPaError();

    if( stream )
    {
        err = Pa_StartStream( stream );
        checkPaError();

        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
        checkPaError();

        err = Pa_CloseStream( stream );
        checkPaError();

        printf("Done.\n"); fflush(stdout);
    }
}
