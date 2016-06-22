#include <stdlib.h>
#include <stdio.h>

#include <portaudio.h>

typedef struct
{
    float left_phase;
    float right_phase;
}
paTestData;
/* This routine will be called by the PortAudio engine when audio is needed.
   It may called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{

}

int main()
{
    PaError err;

    err = Pa_Initialize();
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));

    /*#define SAMPLE_RATE (44100)
    static paTestData data;

    PaStream *stream;
    // Open an audio I/O stream.
    err = Pa_OpenDefaultStream( &stream,
                                0,          // no input channels
                                2,          // stereo output
                                paFloat32,  // 32 bit floating point output
                                SAMPLE_RATE,
                                256,        // frames per buffer, i.e. the number of sample frames that PortAudio will request from the callback.
                                            // Many apps may want to use paFramesPerBufferUnspecified, which tells PortAudio to pick the best, possibly changing, buffer size.
                                patestCallback, // this is your callback function
                                &data ); // This is a pointer that will be passed to your callback
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));*/

    err = Pa_Terminate();
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));

    return 0;
}
