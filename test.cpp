#include <stdio.h>
#include <stdlib.h>
#include <time.h>       // time_t, struct tm, difftime, time, mktime
#include <cmath>

int main()
{
    tm time2016 = tm();
    time2016.tm_year = 116; time2016.tm_mon = 0; time2016.tm_mday = 1;
    time2016.tm_hour = 0;   time2016.tm_min = 0; time2016.tm_sec = 0;

    time_t current_time = time(NULL);
    int mins = round((current_time - mktime(&time2016))/60.0);
    printf("%d\n", mins);

    return 0;
}
