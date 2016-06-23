#include "wpiengine.hpp"

void WpiEngine::init()
{
    err = Pa_Initialize();
    printf( "\n%s\nversion number %d\n\n", Pa_GetVersionText(), Pa_GetVersion());
    checkPaError();
}

void WpiEngine::terminate()
{
    err = Pa_Terminate();
}

void WpiEngine::checkPaError()
{
    if(err != paNoError)
    {
        printf("PortAudio error %d: %s\n", err, Pa_GetErrorText(err));
        terminate();
    }
}

void WpiEngine::printSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters)
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

void WpiEngine::printDevs()
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
