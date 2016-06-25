#include "wpiengine.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <portaudio.h>

void WpiEngine::init()
{
    PaEngine::init();
    FFTEngine::init();
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
    const SAMPLE*   in      = (const SAMPLE*)   inputBuffer;
    SAMPLE*         out     = (SAMPLE*)         outputBuffer;

    // Prevent unused variable warnings
    (void) timeInfo;
    (void) statusFlags;

    // Play Sine wave
    static size_t frameIndex = 0;
    for( size_t i = 0; i < framesPerBuffer * NUM_CHANNELS; i++ )
        *out++ = wavetable[frameIndex++];
    if( frameIndex > SAMPLE_RATE ) frameIndex -= SAMPLE_RATE;

    // Calculate and display input volume
    SAMPLE volume = 0;
    for( size_t i = 0; i < NUM_CHANNELS * framesPerBuffer; i++ )
        volume += inputBuffer ? *in++ : 0;
    volume /= (NUM_CHANNELS * framesPerBuffer);
    //printf ("%f\n", volume);
    /*for (int i = 0; i<75; i++) printf (" ");
    printf ("\r");
    for (int i = 0; i<volume*250; i++) printf ("|");
    printf ("\r");
    fflush(stdout);*/

    // Write to file
    memcpy ( fftin, inputBuffer, sizeof(SAMPLE) * NUM_CHANNELS * framesPerBuffer );
    hanning();
    fft();
    getFrequency();
    if (fid) fwrite( inputBuffer, sizeof(SAMPLE), NUM_CHANNELS * framesPerBuffer, fid );

    return paContinue;
}

void WpiEngine::windoo()
{
    // Open output file
    if (filename)
        ((fid = fopen(filename, "wb"))) ?
            printf("Opened file '%s' for output.", filename) :
            printf("Could not open file '%s' for output.", filename);

    // Set output parameters
    selectDefaultInputParameters();
    selectDefaultOutputParameters();

    // Record some audio. --------------------------------------------
    err = Pa_OpenStream(
              &stream,
              inputParameters,
              outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,                // we won't output out of range samples so don't bother clipping them
              windooCallbackWrapper,
              this );
    checkPaError();

    err = Pa_StartStream( stream );
    checkPaError();
    printf("\n=== Stream started ===\n"); fflush(stdout);

    // Wait for stream to finish
    getchar();
    err = Pa_StopStream( stream );
    checkPaError();
    printf("\n=== Stream stopped ===\n"); fflush(stdout);

    err = Pa_CloseStream( stream );
    checkPaError();
    printf("\n=== Stream closed ===\n"); fflush(stdout);

    if (fid)
    {
        fclose( fid );
        printf("Wrote data to '%s'\n", filename);
    }
}
