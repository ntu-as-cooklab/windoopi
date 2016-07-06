#include "wpiengine.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <vector>
using std::vector;

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

double frequencyToWindSpeed(double frequency)
{
    //printf("Frequency:       %f\n", frequency);
    double windFrequency = frequency / 20.0;
    double wind = windFrequency * (
    - 3.3857e-13 * pow(windFrequency, 5)
    + 4.3840e-10 * pow(windFrequency, 4)
    - 2.1796e-7  * pow(windFrequency, 3)
    + 5.2009e-5  * pow(windFrequency, 2)
    - 6.0440e-3  * windFrequency
    + 6.6953e-1 );
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

    switch (currentMeasureType)
    {
        case 2: // WIND
        {
            double wind = frequencyToWindSpeed(f);
            if (filterWind(wind))
            {
                Wind += wind;
                nWind ++;
                printf("            Wind speed:  %f\n", wind);
            }
            break;
        }
        case 3: // TEMP
        {
            double temp = frequencyToTemperature(f);
            printf("            Temperature: %f\n", temp);
            break;
        }
        case 4: // HUMIDITY
        {
            double humidity = frequencyToHumidity(f);
            Humidity += humidity;
            nHumidity ++;
            printf("            Humidity:    %f\n", humidity);
            break;
        }
        case 5: // HUMIDITY_TEMP
        {
            double humiditytemp = frequencyToHumidityTemp(f);
            Temperature += humiditytemp;
            nTemperature ++;
            printf("(Humidity)  Temperature: %f\n", humiditytemp);
            break;
        }
        case 6:
        {
            pres1_corr = f;
            break;
        }
        case 7:
        {
            pres2_corr = f;
            double pressure = (100.0 * ((double) round(pres1_corr / 100.0))) + (pres2_corr / 100.0);
            pres1_corr = 0.0;
            pres2_corr = 0.0;
            Pressure += frequencyToWindSpeed(f);
            nPressure ++;
            printf("(Corrected) Pressure:    %f\n", pressure);
            break;
        }
        case 8: // PRESSURE
        {
            printf("            Pressure:    %f\n", frequencyToPressure(f));
            break;
        }
    }
}

bool WpiEngine::filterWind(double value)
{
    if (value >= 0.0 && value <= 150.0) {
        if (smoothingWind < 0) {
            smoothingWind = value;
            return true;
        }
        int indexWindMaxGapValue;
        if (value < 40.0)
            indexWindMaxGapValue = 0;
        else if (value < 60.0)
            indexWindMaxGapValue = 1;
        else if (value < 80.0)
            indexWindMaxGapValue = 2;
        else if (value < 100.0)
            indexWindMaxGapValue = 3;
        else
            indexWindMaxGapValue = 4;

        int gaps[5] = {30, 25, 20, 15, 10};

        bool gapIsTooLarge = fabs(value - smoothingWind) > (double) gaps[indexWindMaxGapValue];
        if (!gapIsTooLarge) {
            countWind = COUNT_WIND_DEFAULT;
            smoothingWind = value;
            return true;
        } else if (countWind <= 0) {
            countWind = COUNT_WIND_DEFAULT;
            smoothingWind = value;
            return false;
        } else if (gapIsTooLarge) {
            countWind--;
            return false;
        }
    }
    return false;
}
