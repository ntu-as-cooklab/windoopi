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

    err = Pa_Terminate();
    if( err != paNoError ) printf("PortAudio error: %s\n", Pa_GetErrorText(err));

    return 0;
}
