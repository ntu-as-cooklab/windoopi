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
