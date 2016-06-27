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

#include "patestengine.hpp"

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
    return ((PaTestEngine*) userData)->recordCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int PaTestEngine::recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags)
{
    const SAMPLE* in = (const SAMPLE*) inputBuffer;

    // Prevent unused variable warnings
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    // Calculate and display current volume
    SAMPLE volume = 0;
    for( size_t i = 0; i < NUM_CHANNELS * framesPerBuffer; i++ )
        volume += inputBuffer ? *in++ : 0;
    volume /= (NUM_CHANNELS * framesPerBuffer);
    //printf ("%f\n", volume);
    for (int i = 0; i<75; i++) printf (" ");
    printf ("\r");
    for (int i = 0; i<volume*250; i++) printf ("|");
    printf ("\r");
    fflush(stdout);

    // Write to file
    if (fid) fwrite( inputBuffer, sizeof(SAMPLE), NUM_CHANNELS * framesPerBuffer, fid );

    return paContinue;
}

void PaTestEngine::record()
{
    // Open output file
    if (filename)
        ((fid = fopen(filename, "wb"))) ?
            printf("Opened file '%s' for output.", filename) :
            printf("Could not open file '%s' for output.", filename);

    // Set output parameters
    selectDefaultInputParameters();

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
    getchar();
    err = Pa_CloseStream( stream );
    checkPaError();

    if (fid)
    {
        fclose( fid );
        printf("Wrote data to '%s'\n", filename);
    }
}
