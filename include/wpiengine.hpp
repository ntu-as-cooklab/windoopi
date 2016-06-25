#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include "paengine.hpp"
#include "fftengine.hpp"

struct WpiEngine : public PaEngine, public FFTEngine
{
    void init();
    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
};

class FFTRealDouble_Mixed;
class FFTRealDouble;

#endif
