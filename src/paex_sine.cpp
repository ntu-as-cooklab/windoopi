/** @file paex_sine.c
	@ingroup examples_src
	@brief Play a sine wave for several seconds.
	@author Ross Bencina <rossb@audiomulch.com>
    @author Phil Burk <philburk@softsynth.com>
*/
/*
 * $Id: paex_sine.c 1752 2011-09-08 03:21:55Z philburk $
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com/
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
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#include <portaudio.h>

#include "wpiengine.hpp"

static int playWaveCallbackWrapper( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    return ((WpiEngine*) userData)->playWaveCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int WpiEngine::playWaveCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags)
{
    float *out = (float*) outputBuffer;

    (void) timeInfo; // Prevent unused variable warnings.
    (void) statusFlags;
    (void) inputBuffer;

    static int left_phase  = 0;
    static int right_phase = 0;

    for( size_t i = 0; i < framesPerBuffer; i++ )
    {
        *out++ = wavetable[left_phase];      // left
        *out++ = wavetable[right_phase];     // right
        left_phase += 1;
        if( left_phase >= TABLE_SIZE )  left_phase -= TABLE_SIZE;
        right_phase += 3;                 // higher pitch so we can distinguish left and right.
        if( right_phase >= TABLE_SIZE ) right_phase -= TABLE_SIZE;
    }

    return paContinue;
}

// This routine is called by portaudio when playback is done.
static void StreamFinished( void* userData )
{
    (void) userData; // Prevent unused variable warnings.
    printf( "Stream Completed.\n");
}

void WpiEngine::playWave()
{
    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

    // Set output parameters
    delete outputParameters;
    outputParameters = new PaStreamParameters;
    if ( (outputParameters->device = Pa_GetDefaultOutputDevice()) == paNoDevice )  return printf("Error: No default output device.\n"), terminate();
    outputParameters->channelCount = NUM_CHANNELS;
    outputParameters->sampleFormat = PA_SAMPLE_TYPE;
    outputParameters->suggestedLatency = Pa_GetDeviceInfo( outputParameters->device )->defaultLowOutputLatency;
    outputParameters->hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL,             // no input
              outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,        // we won't output out of range samples so don't bother clipping them
              playWaveCallbackWrapper,
              this );
    checkPaError();

    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
    checkPaError();

    err = Pa_StartStream( stream );
    checkPaError();

    printf("Play for %d seconds.\n", NUM_SECONDS );
    Pa_Sleep( NUM_SECONDS * 1000 );

    err = Pa_StopStream( stream );
    checkPaError();

    err = Pa_CloseStream( stream );
    checkPaError();
}
