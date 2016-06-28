#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include "paengine.hpp"
#include "fftengine.hpp"
#include <vector>

#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

//#define DITHER_FLAG   (paDitherOff)
typedef short SAMPLE;

struct WpiEngine : public PaEngine, public FFTEngine
{
    double      OUTPUT_FREQUENCY = 15e3;
    SAMPLE*     wavetable           = NULL;

    WpiEngine()
    {
        PA_SAMPLE_TYPE      = paInt16;
        NUM_CHANNELS        = 1;        // mono
        SAMPLE_RATE         = 44100;
        FRAMES_PER_BUFFER   = 2048;
        N                   = 8192;
        genSineWavetable    (15e3);
    }
    ~WpiEngine()
    {
        //delete [] wavetable;
        //wavetable = NULL;
        close(uart0_filestream);
    }

    double F_max()                       { return SAMPLE_RATE / 2.f; }
    double resolution()                  { return SAMPLE_RATE / (double) N; }
    double Lowest_Detectable_Frequency() { return 5 * resolution(); }

    void init();
    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    double getFrequency();

    void genSineWavetable(double frequency);
    void genFloatSineWavetable(double frequency);
    void genEmptyWavetable();

    std::vector<double> header;
    std::vector<double> data;
    int currentMeasureType = 0;
    double calibValue = 1000.0;
    std::vector<double> calibValues;
    double pres1_corr = 0.0;
    double pres2_corr = 0.0;

    int finalizeHeader();
    void finalizeData();

    void initUART();
    int uart0_filestream = -1;
};

#endif
