#include "WpiEngine.hpp"
#include <wiringPi.h>
#include <wiringSerial.h>
#include <cmath>

void WpiEngine::initSerial()
{
    if (wiringPiSetup () == -1)                         printf ("Unable to start wiringPi.\n") ;
    pinMode (1, OUTPUT) ;

    if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)   printf ("Unable to open serial device.\n") ;

    time2016.tm_hour = 0;   time2016.tm_min = 0; time2016.tm_sec = 0;
    time2016.tm_year = 116; time2016.tm_mon = 0; time2016.tm_mday = 1;
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
        unsigned short  thetime         = round( difftime(timer, mktime(&time2016)) * 1000.0 ) ;
        unsigned short  humidity        = round( Humidity / (double) nHumidity * 100.0 ) ;
        short           temperature     = round( Temperature / (double) nTemperature * 100.0 ) ;
        unsigned short  pressure        = round( Pressure / (double) nPressure * 10.0 ) ;
        unsigned short  wind            = round( Wind / (double) nWind * 100.0 ) ;

        char message[34];
        sprintf(message, "AT+DTX=22,");
        sprintf(message+10, "%04X%04X%04X%04X%04X%02X", thetime, humidity, temperature, pressure, wind, 0);
        message[32] = '\r';
        message[33] = '\n';

        for (int i=0; i<34; i++)
            serialPutchar (fd, message[i]);

        printf("Sent to serial: %s\n", message);

        Temperature = Wind = Pressure = Humidity = 0;
        nTemperature = nWind = nPressure = nHumidity = 0;
    }

    /*if (serialDataAvail (fd))
    {
      printf (" --> ");
      printf ("%c", serialGetchar (fd));
      while (serialDataAvail (fd)) printf ("%c", serialGetchar (fd)) ;
      printf ("\n");
    }*/
}
