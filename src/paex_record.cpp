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
static int recordCallbackWrapper( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    return ((WpiEngine*) userData)->recordCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int WpiEngine::recordCallback( const void *inputBuffer, void *outputBuffer,
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

void WpiEngine::record()
{
    // Set input parameters
    delete inputParameters;
    inputParameters = new PaStreamParameters;
    if ( (inputParameters->device = Pa_GetDefaultInputDevice()) == paNoDevice )  return printf("Error: No default input device.\n"), terminate();
    inputParameters->channelCount = NUM_CHANNELS;
    inputParameters->sampleFormat = PA_SAMPLE_TYPE;
    inputParameters->suggestedLatency = Pa_GetDeviceInfo( inputParameters->device )->defaultLowInputLatency;
    inputParameters->hostApiSpecificStreamInfo = NULL;

    // Initialize data
    initSampleData();

    // Record some audio. --------------------------------------------
    err = Pa_OpenStream(
              &stream,
              inputParameters,
              NULL,                     //&outputParameters
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,                // we won't output out of range samples so don't bother clipping them
              recordCallbackWrapper,
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
    SAMPLE max = 0, average = 0;
    for(size_t i = 0; i < numSamples(); i++ )
    {
        SAMPLE val = sampleData[i];
        if( val < 0 )   val = -val; // ABS
        if( val > max ) max = val;
        average += val;
    }
    average /= numSamples();
    printf("Sample max amplitude = " PRINTF_S_FORMAT "\n", max );
    printf("Sample average       = " PRINTF_S_FORMAT "\n", average );
}

void WpiEngine::write()
{
    // Write recorded data to a file.
    {
        FILE  *fid;
        if( (fid = fopen("recorded.raw", "wb")) == NULL ) printf("Could not open file.");
        else
        {
            fwrite( sampleData, NUM_CHANNELS * sizeof(SAMPLE), numFrames(), fid );
            fclose( fid );
            printf("Wrote data to 'recorded.raw'\n");
        }
    }
}

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
    SAMPLE  *rptr           = & sampleData[frameIndex * NUM_CHANNELS];
    SAMPLE  *wptr           = (SAMPLE*) outputBuffer;
    int     finished;

    (void) inputBuffer; // Prevent unused variable warnings.
    (void) timeInfo;
    (void) statusFlags;

    if( size_t framesLeft = numFrames() - frameIndex < framesPerBuffer )
    {
        // final buffer...
        for( size_t i = 0; i < framesPerBuffer; i++ )
            for (unsigned int n = 0; n < NUM_CHANNELS; n++) *wptr++ = i < framesLeft ? *rptr++ : 0;
        frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( size_t i = 0; i < framesPerBuffer; i++ )
            for (unsigned int n = 0; n < NUM_CHANNELS; n++) *wptr++ = *rptr++;
        frameIndex += framesPerBuffer;
        finished = paContinue;
    }
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
