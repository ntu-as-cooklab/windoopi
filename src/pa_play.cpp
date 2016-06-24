#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    static  SAMPLE*     buffer  = new SAMPLE[NUM_CHANNELS * framesPerBuffer];
            SAMPLE*     out     = (SAMPLE*) outputBuffer;
            SAMPLE*     in      = buffer;

    // Prevent unused variable warnings.
    (void) inputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    bool finish = fread( in, sizeof(SAMPLE), NUM_CHANNELS * framesPerBuffer, fid ) < NUM_CHANNELS * framesPerBuffer;
    memcpy(out, in, sizeof(SAMPLE) * NUM_CHANNELS * framesPerBuffer);

    // Calculate and display current volume
    SAMPLE volume = 0;
    for( size_t i = 0; i < NUM_CHANNELS * framesPerBuffer; i++ )
        volume += *in++;
    volume /= (NUM_CHANNELS * framesPerBuffer);
    for (int i = 0; i<75; i++) printf (" ");
    printf ("\r");
    for (int i = 0; i<volume*250; i++) printf ("|");
    printf ("\r");
    fflush(stdout);

    if (finish) delete [] buffer;

    return finish ? paComplete : paContinue;
}

void WpiEngine::play()
{
    // Open input file
    if (filename)
    {
        if ((fid = fopen(filename, "rb"))) printf("Opened file '%s' for input.", filename);
        else return printf("Could not open file '%s' for input.", filename), terminate();
    }

    // Set output parameters
    selectDefaultOutputParameters();

    printf("\n=== Now playing back. ===\n"); fflush(stdout);
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
    }

    if (fid)
        fclose( fid );

    printf("Done.\n"); fflush(stdout);
}
