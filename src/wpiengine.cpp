#include "wpiengine.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#include <math.h>
#include <portaudio.h>
#include <algorithm>
#include <vector>
using std::vector;

#include <wiringPi.h>
#include <wiringSerial.h>
#include <time.h>       // time_t, struct tm, difftime, time, mktime

void WpiEngine::init()
{
    printf( "\n========== Windoo Testing ==========\n\n");
    PaEngine::init();
    FFTEngine::init();
    initSerial();
    printf( "Size of sample:              %d bits\n", sizeof(SAMPLE) * 8);
    printf( "Number of channels:          %d\n", NUM_CHANNELS);
    printf( "Sample rate:                 %d\n", SAMPLE_RATE);
    printf( "FFT size:                    %d\n", N);
    printf( "Frequency resolution:        %f\n", resolution());
    printf( "Lowest detectable frequency: %f\n", Lowest_Detectable_Frequency());
    printf( "Output frequency:            %f\n", OUTPUT_FREQUENCY);

    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;


    y2k.tm_year = 115; y2k.tm_mon = 0; y2k.tm_mday = 1;
}

static int windooCallbackWrapper( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    return ((WpiEngine*) userData)->windooCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int WpiEngine::windooCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags)
{
    static SAMPLE*  in      = (SAMPLE*)         fftin;
    SAMPLE*         out     = (SAMPLE*)         outputBuffer;

    // Prevent unused variable warnings
    (void) timeInfo;
    (void) statusFlags;

    // Play Sine wave
    static size_t frameIndex = 0;
    for( size_t i = 0; i < framesPerBuffer; i++ )
    {
        *out++ = wavetable[frameIndex++];
        if( frameIndex >= SAMPLE_RATE ) frameIndex -= SAMPLE_RATE;
    }

    // FFT
    memcpy ( in, inputBuffer, sizeof(SAMPLE) * framesPerBuffer );
    in += framesPerBuffer;
    if (in - fftin > N)
    {
        hanning();
        fft();
        double f = getFrequency();
        //printf("%f\n", f);
        if (f >= 100. && f < 900.0)
        {
            header.push_back(f);
            if (currentMeasureType != 0 && data.size() > 0)
            {
                finalizeData();
                data.clear();
            }
        }
        else if (f >= 900.0 && f <= 20000.0)
        {
            data.push_back(f);
            if (header.size() > 0)
            {
                currentMeasureType = finalizeHeader();
                header.clear();
            }
        }
        in = fftin;
    }

    serialWrite();

    // Write to file
    if (fid) fwrite( inputBuffer, sizeof(SAMPLE), framesPerBuffer, fid );

    return paContinue;
}

char message[44];

void WpiEngine::initSerial()
{
    if ((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0)
        printf ("Unable to open serial device: %s\n") ;

    if (wiringPiSetup () == -1)
        printf ("Unable to start wiringPi: %s\n") ;

    sprintf(message, "AT+DTX=22,");
    message[42] = '\r';
    message[43] = '\n';
}

void WpiEngine::serialWrite()
{
    static unsigned int lastTime  = millis();

    unsigned int thisTime = millis();
    if ( thisTime - lastTime > 60e3 )
    {
        lastTime = thisTime;
        time_t timer;
        time(&timer);  // get current time; same as: timer = time(NULL)
        unsigned short  thetime         = round( difftime(timer, mktime(&y2k)) * 1000.0 ) ;
        unsigned short  humidity        = round( Humidity / (double) nHumidity * 100.0 ) ;
        short           temperature     = round( Temperature / (double) nTemperature * 100.0 ) ;
        unsigned short  pressure        = round( Pressure / (double) nPressure * 10.0 ) ;
        unsigned short  wind            = round( Wind / (double) nWind * 100.0 ) ;

        /**((unsigned short*) ((void*) message + 10))  = thetime;
        *((unsigned short*) ((void*) message + 12))  = humidity;
        *((unsigned short*) ((void*) message + 14))  = temperature;
        *((unsigned short*) ((void*) message + 16))  = pressure;
        *((unsigned short*) ((void*) message + 18))  = wind;
        *((char*) ((void*) message + 20))           = 0;

        //printf("\n");*/

        sprintf(message+10, "%04X%04X%04X%04X%04X%02X", thetime, humidity, temperature, pressure, wind, 0);

        printf("Send to serial: ");
        for (int i=0; i<44; i++)
        {
            printf("%c", message[i]);
            serialPutchar (fd, message[i]);
        }
        printf("\n");


        Temperature = Wind = Pressure = Humidity = 0;
        nTemperature = nWind = nPressure = nHumidity = 0;
    }
}

void WpiEngine::genSineWavetable(double frequency)
{
    delete [] wavetable;
    wavetable = new short[SAMPLE_RATE];
    char* generatedSnd = (char*) wavetable;
    double* sample = new double[SAMPLE_RATE];
    for (int i = 0; i < SAMPLE_RATE; i++)
        sample[i] = sin((2.0 * M_PI * ((double) i)) / (SAMPLE_RATE / frequency));

    int idx = 0;
    for (int i = 0; i < SAMPLE_RATE; i++)
    {
        double dVal = sample[i];
        short val = (short) ((int) (32767.0 * dVal));
        int i2 = idx + 1;
        generatedSnd[idx] = (char) (val & 255);
        idx = i2 + 1;
        generatedSnd[i2] = (char) ((65280 & val) >> 8);
    }
    delete [] sample;
}

void WpiEngine::genFloatSineWavetable(double frequency)
{
    // initialise sinusoidal wavetable
    OUTPUT_FREQUENCY = frequency;
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[SAMPLE_RATE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    for( size_t i = 0; i < SAMPLE_RATE; i++ )
        wavetable[i] =  (float) sin( 2.f * M_PI * ((double)i) / ((double)SAMPLE_RATE) * frequency );
}

void WpiEngine::genEmptyWavetable()
{
    delete [] wavetable;
    wavetable = NULL;
    if ( ! (wavetable = new SAMPLE[SAMPLE_RATE]) )  return printf("Could not allocate wavetable array.\n"), terminate();
    memset (wavetable, 0, SAMPLE_RATE * sizeof(SAMPLE));
}

void WpiEngine::windoo()
{
    // Open output file
    if (filename)
        ((fid = fopen(filename, "wb"))) ?
            printf("Opened file '%s' for output.", filename) :
            printf("Could not open file '%s' for output.", filename);

    // Set output parameters
    selectDefaultInputParameters();
    selectDefaultOutputParameters();

    // Record some audio. --------------------------------------------
    err = Pa_OpenStream(
              &stream,
              inputParameters,
              outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,                // we won't output out of range samples so don't bother clipping them
              windooCallbackWrapper,
              this );
    checkPaError();

    err = Pa_StartStream( stream );
    checkPaError();
    printf("=== Stream started ===\n"); fflush(stdout);

    // Wait for stream to finish
    getchar();
    err = Pa_StopStream( stream );
    checkPaError();
    printf("=== Stream stopped ===\n"); fflush(stdout);

    err = Pa_CloseStream( stream );
    checkPaError();
    printf("=== Stream closed ===\n"); fflush(stdout);

    if (fid)
    {
        fclose( fid );
        printf("Wrote data to '%s'\n", filename);
    }
}

inline double magnitude(fftw_complex z)
{
    return sqrt(z[0]*z[0] + z[1]*z[1]);
}

double WpiEngine::getFrequency()
{
    const int A = 1;
    int bin[A];
    double max[A];
    for (int a = 0; a < A; a++)
    {
        bin[a] = 0;
        max[a] = 0.f;
    }

    for (int i = 0; i < N_bins(); i++)
        for (int a = 0; a < A; a++)
            if ( magnitude(fftout[i]) > max[a] && (resolution() * i) < 20e3)
            {
                if (a+1 < A)
                {
                    bin[a+1] = bin[a];
                    max[a+1] = max[a];
                }
                bin[a] = i;
                max[a] = magnitude(fftout[i]);
                break;
            }

    /*printf ("---------- Input frequency strengths ----------\n");
    for (int a = 0; a < A; a++)
    {
        double freq = resolution() * bin[a];
        printf ("[%2d]\tFreq: %12.6f,\tAmp: %12.6f\n", a+1, freq, max[a]);
    }*/

    /*int BinsPerBar = 256;
    for (int i = 0; i < N_bins()/BinsPerBar; i++)
    {
        float freq = resolution() * (i*BinsPerBar + BinsPerBar/2.0);
        float amp = 0;
        for (int n = 0; n < BinsPerBar; n++) amp += magnitude(fftout[i*BinsPerBar + n]);
        if (freq < 20e3)
        {
            printf ("%10.3f: %15.2f", freq, 20 * log(amp));
            if (freq, 20 * log(amp) > 300) printf("*****");
            printf("\n");
        }
    }*/
    return resolution() * bin[0];
}

int WpiEngine::finalizeHeader()
{
        std::sort(header.begin(), header.begin() + header.size());
        double f = header[header.size() / 2];
        if (f >= 480.0 && f <= 520.0)
            return 1;
        if (f >= 530.0 && f <= 570.0)
            return 2;
        if (f >= 580.0 && f <= 620.0)
            return 3;
        if (f >= 630.0 && f <= 670.0)
            return 4;
        if (f >= 680.0 && f <= 720.0)
            return 5;
        if (f >= 730.0 && f <= 770.0)
            return 6;
        if (f < 780.0 || f > 820.0)
            return 0;
        return 7;
}

inline double frequencyToWindSpeed(double frequency)
{
    double windFrequency = frequency / 20.0;
    double wind = windFrequency * (((((((-3.3857 * pow(10.0, -13.0)) * pow(windFrequency, 5.0)) + ((4.384 * pow(10.0, -10.0)) * pow(windFrequency, 4.0))) - ((2.1796 * pow(10.0, -7.0)) * pow(windFrequency, 3.0))) + ((5.2009 * pow(10.0, -5.0)) * pow(windFrequency, 2.0))) - ((6.044 * pow(10.0, -3.0)) * windFrequency)) + (6.6953 * pow(10.0, -1.0)));
    if (wind < 1.0)
        return 0.0;
    return wind;
}

inline double frequencyToTemperature(double frequency)
{
    return (3380.0 / log((10.0 * frequency) / (10000.0 * exp(-11.336575549220193)))) - 273.15;
}

inline double frequencyToHumidity(double frequency)
{
    return -6.0 + ((125.0 / pow(2.0, 16.0)) * (frequency * 4.0));
}

inline double frequencyToHumidityTemp(double frequency)
{
    return -46.85 + ((175.72 / pow(2.0, 16.0)) * (frequency * 4.0));
}

inline double frequencyToPressure(double frequency)
{
    return frequency / 10.0;
}

void WpiEngine::finalizeData()
{
    std::sort(data.begin(), data.begin() + data.size());
    double f = data[data.size() / 2];
    //if (getStandardDeviation(data) >= 1000.0) {}

    if (currentMeasureType == 1)
    {
        calibValues.push_back(f);
        printf("Calibrating = TRUE\n");
        return;
    }
    if (currentMeasureType != 1 && calibValues.size() > 0)
    {
        std::sort(calibValues.begin(), calibValues.begin() + calibValues.size());
        calibValue = calibValues[calibValues.size() / 2];
        calibValues.clear();
        printf("Calibrating = FALSE\n");
    }

    f = ((f / calibValue) * 1000.0) - 1000.0;
    if (currentMeasureType == 6)
        pres1_corr = f;
    else if (currentMeasureType == 7)
    {
        pres2_corr = f;
        double pressure = (100.0 * ((double) round(pres1_corr / 100.0))) + (pres2_corr / 100.0);
        pres1_corr = 0.0;
        pres2_corr = 0.0;
        Pressure += frequencyToWindSpeed(f);
        nPressure ++;
        printf("Pressure: %f\n", pressure);
    }
    else
    {
        switch (currentMeasureType)
        {
            case 2: // WIND
                printf("Wind: %f\n", frequencyToWindSpeed(f));
                Wind += frequencyToWindSpeed(f);
                nWind ++;
                break;
            case 3: // TEMP
                printf("Temp: %f\n", frequencyToTemperature(f));
                Temperature += frequencyToTemperature(f);
                nTemperature ++;
                break;
            case 4: // HUMIDITY
                printf("Humidity: %f\n", frequencyToHumidity(f));
                break;
            case 5: // HUMIDITY_TEMP
                printf("Humidity/Temp: %f\n", frequencyToHumidityTemp(f));
                Humidity += frequencyToHumidityTemp(f);
                nHumidity ++;
                break;
            case 8: // PRESSURE
                printf("Pressure: %f\n", frequencyToPressure(f));
                break;
        }
    }
}
