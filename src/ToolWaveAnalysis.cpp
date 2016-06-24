struct ToolWaveAnalysis
{
private:
    const int DATA_HUMIDITY_MAX = 15000;
    const int DATA_HUMIDITY_MIN = 1900;
    const int DATA_HUMIDITY_TEMP_MAX = 11000;
    const int DATA_HUMIDITY_TEMP_MIN = 3500;
    const int DATA_MAX = 20000;
    const int DATA_MIN = 900;
    const int DATA_PRESSURE_MAX = 12000;
    const int DATA_PRESSURE_MIN = 3000;
    const int DATA_TEMP_MAX = 9000;
    const int DATA_TEMP_MIN = 1400;
    const int DATA_WIND_MAX = 4000;
    const int DATA_WIND_MIN = 1020;
    const int HEADER_CALIB = 500;
    const int HEADER_CALIB_MAX = 520;
    const int HEADER_CALIB_MIN = 480;
    const int HEADER_HUMIDITY = 650;
    const int HEADER_HUMIDITY_MAX = 670;
    const int HEADER_HUMIDITY_MIN = 630;
    const int HEADER_HUMIDITY_TEMP = 700;
    const int HEADER_HUMIDITY_TEMP_MAX = 720;
    const int HEADER_HUMIDITY_TEMP_MIN = 680;
    const int HEADER_MAX = 900;
    const int HEADER_MIN = 100;
    const int HEADER_PRESSURE_1 = 750;
    const int HEADER_PRESSURE_1_MAX = 770;
    const int HEADER_PRESSURE_1_MIN = 730;
    const int HEADER_PRESSURE_2 = 800;
    const int HEADER_PRESSURE_2_MAX = 820;
    const int HEADER_PRESSURE_2_MIN = 780;
    const int HEADER_TEMP = 600;
    const int HEADER_TEMP_MAX = 620;
    const int HEADER_TEMP_MIN = 580;
    const int HEADER_WIND = 550;
    const int HEADER_WIND_MAX = 570;
    const int HEADER_WIND_MIN = 530;
    const int LIMITS = 20;

    int bufferSize = 2048;
    double calibValue = 1000.0;
    double* calibValues = new ArrayList<Double>();
    int fftBins = 32768;
    int lastHumidityGenerated;
    int lastHumidityTempGenerated;
    int lastPressureGenerated;
    int lastTempGenerated;
    int lastWindGenerated;
    double pres1_corr = 0.0;
    double pres2_corr = 0.0;
    int sampleRate = 44100;

    double getFrequency(double* fftData)
    {
        double maxVal = -1.0;
        int maxIndex = -1;
        for (int j = 0; j < fftData.length / 2; j++)
        {
            double re = fftData[j * 2];
            double im = fftData[(j * 2) + 1];
            double magnitude = Math.sqrt((re * re) + (im * im));
            if (magnitude > maxVal)
            {
                maxIndex = j;
                maxVal = magnitude;
            }
        }
        return ((double) (this.sampleRate * maxIndex)) / ((double) fftData.length);
    }

public:

    void run()
    {
        int currentMeasureType = 0;
        double* data;
        double* header;

        FFTRealDouble fft = new FFTRealDouble(fftBins);
        double[] fftData = new double[fftBins];

        while (isRunning())
        {
            audioRecord.read(fftData, bufferSize);
            fft.ft(fftData);
            double f = getFrequency(fftData);
            if (f >= 100.0 && f < 900.0)
            {   // header
                header.add(f);
                if (currentMeasureType != 0 && data.size() > 0)
                {
                    finalizeData(currentMeasureType, data);
                    data.clear();
                }
            }
            else if (f >= 900.0 && f <= 20000.0)
            {   // data
                data.add(f);
                if (header.size() > 0)
                {
                    currentMeasureType = finalizeHeader(header);
                    header.clear();
                }
            }
        }
    }

    private int finalizeHeader(double* header)
    {
        Collections.sort(header);
        double f = header[header.size() / 2]; // get median frequency
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

    private void finalizeData(int currentMeasureType, ArrayList<Double> data)
    {
        Collections.sort(data);
        double f = data.get(data.size() / 2); // get median frequency

        if (getStandardDeviation(data) >= 1000.0) {
        }
        if (currentMeasureType == 1) {
            this.calibValues.add(f);
            setChanged();
            notifyObservers(new EventDetectCalibration(1, true));
            return;
        }
        if (currentMeasureType != 1 && this.calibValues.size() > 0) {
            Collections.sort(this.calibValues);
            this.calibValue = this.calibValues.get(this.calibValues.size() / 2);
            this.calibValues.clear();
            setChanged();
            notifyObservers(new EventDetectCalibration(1, false));
        }
        f = ((f / this.calibValue) * 1000.0d) - 1000.0d;
        if (currentMeasureType == 6) {
            this.pres1_corr = f;
        } else if (currentMeasureType == 7) {
            this.pres2_corr = f;
            Double pressure = (100.0d * ((double) Math.round(this.pres1_corr / 100.0d))) + (this.pres2_corr / 100.0d);
            this.pres1_corr = 0.0d;
            this.pres2_corr = 0.0d;
            setChanged();
            notifyObservers(new EventDetectFrequency(8, pressure.doubleValue()));
        } else {
            setChanged();
            notifyObservers(new EventDetectFrequency(currentMeasureType, f));
        }
    }

    private double getAverage(ArrayList<Double> values) {
        double sum;
        Iterator i$;
        if (values.size() >= 4) {
            sum = 0.0d;
            int i = 0;
            i$ = values.iterator();
            while (i$.hasNext()) {
                double a = (Double) i$.next();
                if (!(i == 0 || i == values.size() - 1)) {
                    sum += a;
                }
                i++;
            }
            return sum / ((double) (values.size() - 2));
        }
        sum = 0.0d;
        i$ = values.iterator();
        while (i$.hasNext()) {
            sum += (Double) i$.next();
        }
        return sum / ((double) values.size());
    }

    private double getStandardDeviation(ArrayList<Double> values) {
        double sum = 0.0d;
        Iterator i$ = values.iterator();
        while (i$.hasNext()) {
            sum += (Double) i$.next();
        }
        double mean = sum / ((double) values.size());
        sum = 0.0d;
        i$ = values.iterator();
        while (i$.hasNext()) {
            double a = (Double) i$.next();
            sum += (mean - a) * (mean - a);
        }
        return Math.sqrt(sum) / ((double) values.size());
    }

    private int random(int lower, int higher) {
        return ((int) (Math.random() * ((double) (higher - lower)))) + lower;
    }

    private int generateNextValue(Integer lastValue, int min, int max) {
        if (lastValue == null) {
            return random(min, max);
        }
        int diff = random(-25, 25);
        if (lastValue + diff < min || lastValue + diff > max) {
            return lastValue + (-diff);
        }
        return lastValue + diff;
    }

};
