#include "WpiEngine.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#include <cmath>
#include <portaudio.h>

#include <time.h>       // time_t, struct tm, difftime, time, mktime

void WpiEngine::init()
{
    printf( "\n========== Windoo Daemon ==========\n\n");
    PaEngine::init();
    printf( "Size of sample:              %d bits\n", sizeof(SAMPLE) * 8);
    printf( "Number of channels:          %d\n", NUM_CHANNELS);
    printf( "Sample rate:                 %d\n", SAMPLE_RATE);
    printf( "FFT size:                    %d\n", N);
    printf( "Frequency resolution:        %f\n", resolution());
    printf( "Lowest detectable frequency: %f\n", Lowest_Detectable_Frequency());
    printf( "Output frequency:            %f\n", OUTPUT_FREQUENCY);

    FFTEngine::init();
    initSerial();
    WPiCURL::init();
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
    {
        *out++ = wavetable[frameIndex++];
        if( frameIndex >= SAMPLE_RATE ) frameIndex -= SAMPLE_RATE;
    }

    // FFT
    memcpy ( in, inputBuffer, sizeof(SAMPLE) * framesPerBuffer );
    in += framesPerBuffer;
    if (in - fftin > N)
    {
        hanning();
        fft();
        double f = getFrequency();

        if (f >= 100. && f < 900.0)
        {
            header.push_back(f);
            if (currentMeasureType != 0 && data.size() > 0)
            {
                finalizeData();
                data.clear();
            }
        }
        else if (f >= 900.0 && f <= 20000.0)
        {
            data.push_back(f);
            if (header.size() > 0)
            {
                currentMeasureType = finalizeHeader();
                header.clear();
            }
        }
        in = fftin;
    }

    // Write to file
    //if (fid) fwrite( inputBuffer, sizeof(SAMPLE), framesPerBuffer, fid );

    return paContinue;
}

void WpiEngine::genSineWavetable(double frequency)
{
    delete [] wavetable;
    wavetable = new short[SAMPLE_RATE];
    char* generatedSnd = (char*) wavetable;
    double* sample = new double[SAMPLE_RATE];
    for (size_t i = 0; i < SAMPLE_RATE; i++)
        sample[i] = sin((2.0 * M_PI * ((double) i)) / (SAMPLE_RATE / frequency));

    int idx = 0;
    for (size_t i = 0; i < SAMPLE_RATE; i++)
    {
        double dVal = sample[i];
        short val = (short) ((int) (32767.0 * dVal));
        int i2 = idx + 1;
        generatedSnd[idx] = (char) (val & 255);
        idx = i2 + 1;
        generatedSnd[i2] = (char) ((65280 & val) >> 8);
    }
    delete [] sample;
}

void WpiEngine::windoo()
{
    // Open output file
    if (filename)
        ((fid = fopen(filename, "w"))) ?
            printf("Opened file '%s' for output.\n", filename) :
            printf("Could not open file '%s' for output.\n", filename);

    // Set output parameters
    selectDefaultInputParameters();
    selectDefaultOutputParameters();
    inputParameters->device = 2;
    outputParameters->device = 2;

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
    while(1)
    {
        static bool high = true;
        digitalWrite (1, high ? HIGH: LOW);
        high = ! high;
        delay(400);
        serialWrite();
        fflush (stdout);
    }

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
