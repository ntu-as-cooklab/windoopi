/** @file paex_record.c
	@ingroup examples_src
	@brief Record input into an array; Save array to a file; Playback recorded data.
	@author Phil Burk  http://www.softsynth.com
*/
/*
 * $Id: paex_record.c 1752 2011-09-08 03:21:55Z philburk $
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.劉
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#include "wpiengine.hpp"

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
            WpiEngine       *engine         = (WpiEngine*) userData;
    const   SAMPLE          *rptr           = (const SAMPLE*) inputBuffer;
            SAMPLE          *wptr           = &engine->sampleData[engine->frameIndex * engine->NUM_CHANNELS];
            long            framesToCalc;
            int             finished;

    // Prevent unused variable warnings
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( unsigned long framesLeft = engine->numFrames() - engine->frameIndex < framesPerBuffer )
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
    static int cumulatedFrames = 0;

    for(long i=0; i<framesToCalc; i++ )
        for (int n=0; n<engine->NUM_CHANNELS; n++)
            cumulatedVolume += (*wptr++ = inputBuffer ? *rptr++ : SAMPLE_SILENCE);
    engine->frameIndex += framesToCalc;

    // Calculate and display current volume
    cumulatedFrames += framesToCalc;
    if (cumulatedFrames >= engine->SAMPLE_RATE/100)
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


void WpiEngine::record()
{
    // Set input parameters
    if ( (inputParameters.device = Pa_GetDefaultInputDevice()) == paNoDevice )  return printf("Error: No default input device.\n"), terminate();
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    // Initialize data
    initSampleData();

    // Record some audio. --------------------------------------------
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,                     //&outputParameters
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,                // we won't output out of range samples so don't bother clipping them
              recordCallback,
              this );
    checkPaError();

    err = Pa_StartStream( stream );
    checkPaError();
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    // Wait for stream to finish
    while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
        Pa_Sleep(100);
    if( err < 0 ) return terminate();

    err = Pa_CloseStream( stream );
    checkPaError();

    // Measure maximum peak amplitude.
    /*max = 0;
    average = 0.0;
    for( i=0; i<numSamples; i++ )
    {
        val = data.recordedSamples[i];
        if( val < 0 )   val = -val; // ABS
        if( val > max ) max = val;
        average += val;
    }
    average = average / (double)numSamples;
    printf("sample max amplitude = "PRINTF_S_FORMAT"\n", max );
    printf("sample average = %lf\n", average );*/
}


/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
/*
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paTestData *data = (paTestData*) userData;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = data->numFrames() - data->frameIndex;

    (void) inputBuffer; // Prevent unused variable warnings.
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        // final buffer...
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  // left
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  // right
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  // left
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  // right
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  // left
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  // right
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

int main ()
{
    PaStreamParameters  outputParameters;
    PaStream*           stream;
    paTestData          data;
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;
    SAMPLE              max, val;
    double              average;

    // Write recorded data to a file.
#if WRITE_TO_FILE
    {
        FILE  *fid;
        fid = fopen("recorded.raw", "wb");
        if( fid == NULL )
            printf("Could not open file.");
        else
        {
            fwrite( data.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), totalFrames, fid );
            fclose( fid );
            printf("Wrote data to 'recorded.raw'\n");
        }
    }
#endif

    Pa_Sleep(3000);

    // Playback recorded data.  --------------------------------------------
    data.frameIndex = 0;

    outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = 2;                     // stereo output
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("\n=== Now playing back. ===\n"); fflush(stdout);
    err = Pa_OpenStream(
              &stream,
              NULL, // no input
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      // we won't output out of range samples so don't bother clipping them
              playCallback,
              &data );
    if( err != paNoError ) goto done;

    if( stream )
    {
        err = Pa_StartStream( stream );
        if( err != paNoError ) goto done;

        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
        if( err < 0 ) goto done;

        err = Pa_CloseStream( stream );
        if( err != paNoError ) goto done;

        printf("Done.\n"); fflush(stdout);
    }

done:
    Pa_Terminate();
    if( data.recordedSamples )       // Sure it is NULL or valid.
        free( data.recordedSamples );
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          // Always return 0 or 1, but no other return codes.
    }
    return err;
}
*/
