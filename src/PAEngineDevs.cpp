/** @file pa_devs.c
	@ingroup examples_src
    @brief List available devices, including device information.
	@author Phil Burk http://www.softsynth.com

    @note Define PA_USE_ASIO=0 to compile this code on Windows without
        ASIO support.
*/
/*
 * $Id: pa_devs.c 1891 2013-05-05 14:00:02Z rbencina $
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
#include <stdlib.h>
#include <cmath>
#include <portaudio.h>

#ifdef WIN32
    #include <windows.h>
    #if PA_USE_ASIO
        #include <pa_asio.h>
    #endif
#endif

#include "paengine.hpp"

void PaEngine::printSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 // negative terminated list
    };

    int printCount = 0;
    for( int i = 0; standardSampleRates[i] > 0; i++ )
    {
        if( (err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i])) == paFormatIsSupported )
        {
            if ( !(printCount%4) ) printf( printCount ? "\n\t" : "\t");
            printf( "%8.2f ", standardSampleRates[i] );
            ++printCount;
        }
    }
    if( !printCount ) printf( "\tNone" );
    printf( "\n" );
}

void PaEngine::printDevs()
{
    int                     numDevices                          = Pa_GetDeviceCount();
    const PaDeviceInfo      *deviceInfo;
    PaStreamParameters      inputParameters, outputParameters;

    printf( "Number of devices = %d\n\n", numDevices );

    for(int i=0; i<numDevices; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);

        printf( "--------------------------------------------------\n" );
        #ifdef WIN32
                // Use wide char on windows, so we can show UTF-8 encoded device names
                wchar_t wideName[MAX_PATH];
                MultiByteToWideChar(CP_UTF8, 0, deviceInfo->name, -1, wideName, MAX_PATH-1);
                wprintf (L"-- Device #%2d                  \"%s\"\n",          i, wideName );
        #else
                printf  ( "-- Device #%2d                  \"%s\"\n",          i, deviceInfo->name );
        #endif

        if ( i == Pa_GetDefaultInputDevice() )
            printf      ( "-- **Default input**\n" );
        else if( i == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultInputDevice )
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
            printf      ( "-- **Default %s input**\n", hostInfo->name );
        }

        if ( i == Pa_GetDefaultOutputDevice() )
            printf      ( "-- **Default output**\n" );
        else if ( i == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice )
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
            printf      ( "-- **Default %s output**\n", hostInfo->name );
        }

        printf          ( "-- Host API                    = %s\n",          Pa_GetHostApiInfo( deviceInfo->hostApi )->name );
        printf          ( "-- Max inputs                  = %d\n",          deviceInfo->maxInputChannels  );
        printf          ( "-- Max outputs                 = %d\n",          deviceInfo->maxOutputChannels  );

        printf          ( "-- Default low input latency   = %8.4f\n",       deviceInfo->defaultLowInputLatency  );
        printf          ( "-- Default low output latency  = %8.4f\n",       deviceInfo->defaultLowOutputLatency  );
        printf          ( "-- Default high input latency  = %8.4f\n",       deviceInfo->defaultHighInputLatency  );
        printf          ( "-- Default high output latency = %8.4f\n",       deviceInfo->defaultHighOutputLatency  );

        #ifdef WIN32
            #if PA_USE_ASIO
            // ASIO specific latency information
                if( Pa_GetHostApiInfo( deviceInfo->hostApi )->type == paASIO )
                {
                    long minLatency, maxLatency, preferredLatency, granularity;
                    err = PaAsio_GetAvailableLatencyValues( i, &minLatency, &maxLatency, &preferredLatency, &granularity );
                    printf( "-- ASIO minimum buffer size    = %ld\n", minLatency  );
                    printf( "-- ASIO maximum buffer size    = %ld\n", maxLatency  );
                    printf( "-- ASIO preferred buffer size  = %ld\n", preferredLatency  );
                    if( granularity == -1 )
                        printf( "-- ASIO buffer granularity     = power of 2\n" );
                    else
                        printf( "-- ASIO buffer granularity     = %ld\n", granularity  );
                }
                #endif
        #endif

        printf          ( "-- Default sample rate         = %8.2f\n",       deviceInfo->defaultSampleRate  );

        // poll for standard sample rates
        inputParameters.device = i;
        inputParameters.channelCount = deviceInfo->maxInputChannels;
        inputParameters.sampleFormat = paInt16;
        inputParameters.suggestedLatency = 0; // ignored by Pa_IsFormatSupported()
        inputParameters.hostApiSpecificStreamInfo = NULL;

        outputParameters.device = i;
        outputParameters.channelCount = deviceInfo->maxOutputChannels;
        outputParameters.sampleFormat = paInt16;
        outputParameters.suggestedLatency = 0; // ignored by Pa_IsFormatSupported()
        outputParameters.hostApiSpecificStreamInfo = NULL;

        if( inputParameters.channelCount > 0 )
        {
            printf("Supported standard sample rates\n for half-duplex 16 bit %d channel input = \n", inputParameters.channelCount );
            printSupportedStandardSampleRates( &inputParameters, NULL );
        }

        if( outputParameters.channelCount > 0 )
        {
            printf("Supported standard sample rates\n for half-duplex 16 bit %d channel output = \n", outputParameters.channelCount );
            printSupportedStandardSampleRates( NULL, &outputParameters );
        }

        if( inputParameters.channelCount > 0 && outputParameters.channelCount > 0 )
        {
            printf("Supported standard sample rates\n for full-duplex 16 bit %d channel input, %d channel output = \n", inputParameters.channelCount, outputParameters.channelCount );
            printSupportedStandardSampleRates( &inputParameters, &outputParameters );
        }

        printf("\n");
    }
}
