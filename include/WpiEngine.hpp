#ifndef WPIENGINE_HPP
#define WPIENGINE_HPP

#include "PAEngine.hpp"
#include "FFTEngine.hpp"
#include "WPiCURL.hpp"
#include <vector>
#include <time.h>       // time_t, struct tm, difftime, time, mktime
#include <wiringPi.h>

//#define DITHER_FLAG   (paDitherOff)
typedef short SAMPLE;

struct WpiEngine : public PaEngine, public FFTEngine, public WPiCURL
{
    double      OUTPUT_FREQUENCY = 15e3;
    SAMPLE*     wavetable           = NULL;

    WpiEngine()
    {
        PA_SAMPLE_TYPE      = paInt16;
        NUM_CHANNELS        = 1;        // mono
        SAMPLE_RATE         = 44100;
        FRAMES_PER_BUFFER   = 2048;
        N                   = 4096;
        genSineWavetable    (15e3);
        init();
        user_id     = 1;
        windoo_id   = 1;
        phone_model = "Raspberry Pi (Network)";
    }
    ~WpiEngine()
    {
        digitalWrite (1, LOW);
        delete [] wavetable;
        wavetable = NULL;
    }

    double F_max()                       { return SAMPLE_RATE / 2.f; }
    double resolution()                  { return SAMPLE_RATE / (double) N; }
    double Lowest_Detectable_Frequency() { return 5 * resolution(); }

    void init();
    void windoo();
    int  windooCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    double getFrequency();

    void genSineWavetable(double frequency);

    std::vector<double> header;
    std::vector<double> data;
    int currentMeasureType = 0;

    int finalizeHeader();
    void finalizeData();

    bool filterWind(double value);
    const int COUNT_WIND_DEFAULT = 3;

    int nHumidity = 0, nTemperature = 0, nPressure = 0, nWind = 0;
    double Time = 0, Humidity = 0, Temperature = 0, Pressure = 0, Wind = 0;
    double avgTime = 0, avgHumidity = 0, avgTemperature = 0, avgPressure = 0, avgWind = 0;

    int fd;
    tm time2016 = tm();
    void initSerial();
    void serialWrite();
};

#endif
