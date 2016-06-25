#include "wpiengine.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#include <math.h>
#include <portaudio.h>
#include <algorithm>

void WpiEngine::init()
{
    printf( "\n========== Windoo Testing ==========\n\n");
    PaEngine::init();
    FFTEngine::init();
    //printf( "Suggested window size: %f\n", Window_Size());
    printf( "Number of channels:          %d\n", NUM_CHANNELS);
    printf( "Sample rate:                 %d\n", SAMPLE_RATE);
    printf( "FFT size:                    %d\n", N);
    printf( "Frequency resolution:        %f\n", resolution());
    printf( "Lowest detectable frequency: %f\n", Lowest_Detectable_Frequency());
    printf( "Output frequency:            %f\n", OUTPUT_FREQUENCY);
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
    static SAMPLE*  in      = (SAMPLE*)         fftin;
    SAMPLE*         out     = (SAMPLE*)         outputBuffer;

    // Prevent unused variable warnings
    (void) timeInfo;
    (void) statusFlags;

    // Play Sine wave
    static size_t frameIndex = 0;
    for( size_t i = 0; i < framesPerBuffer; i++ )
        *out++ = wavetable[frameIndex++];
    if( frameIndex >= SAMPLE_RATE ) frameIndex -= SAMPLE_RATE;

    // FFT
    memcpy ( in, inputBuffer, sizeof(SAMPLE) * framesPerBuffer );
    in += framesPerBuffer;
    if (in - fftin > N)
    {
        hanning();
        fft();
        getFrequency();
        in = fftin;
    }

    // Write to file
    if (fid) fwrite( inputBuffer, sizeof(SAMPLE), framesPerBuffer, fid );

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
    printf("=== Stream started ===\n"); fflush(stdout);

    // Wait for stream to finish
    getchar();
    err = Pa_StopStream( stream );
    checkPaError();
    printf("=== Stream stopped ===\n"); fflush(stdout);

    err = Pa_CloseStream( stream );
    checkPaError();
    printf("=== Stream closed ===\n"); fflush(stdout);

    if (fid)
    {
        fclose( fid );
        printf("Wrote data to '%s'\n", filename);
    }
}

inline float magnitude(fftwf_complex z)
{
    return sqrt(z[0]*z[0] + z[1]*z[1]);
}

void WpiEngine::getFrequency()
{
    const int A = 10;
    int bin[A];
    float max[A];
    for (int a = 0; a < A; a++)
    {
        bin[a] = 0;
        max[a] = 0.f;
    }

    for (int i = 0; i < N_bins(); i++)
        for (int a = 0; a < A; a++)
            if ( magnitude(fftout[i]) > max[a] && resolution() * i < 2e4)
            {
                if (a+1 < A)
                {
                    bin[a+1] = bin[a];
                    max[a+1] = max[a];
                }
                bin[a] = i;
                max[a] = magnitude(fftout[i]);
                break;
            }

    printf ("---------- Frequency strengths ----------\n");
    for (int a = 0; a < A; a++)
    {
        float freq = resolution() * bin[a];
        printf ("[%2d]\tFreq: %12.6f,\tAmp: %12.6f\n", a+1, freq, max[a]);
    }
    printf( "\nPress ENTER to stop stream\n\n");
}
