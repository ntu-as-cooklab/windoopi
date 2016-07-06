#ifndef WPICURL_HPP
#define WPICURL_HPP

#include <curl/curl.h>
#include <string>

struct WPiCURL
{
    CURL        *curl;
    CURLcode    res;

    void init();
    void post();
    ~WPiCURL();

    int             user_id             = 0;
    int             windoo_id           = 0;
    std::string     time_start;
    std::string     time_finish;
    double          location_latitude   = 0;
    double          location_longitude  = 0;
    double          location_altitude   = 0;
    double          location_heading    = 0;
    std::string     location_time;
    double          windoo_wind         = 0;
    double          windoo_temperature  = 0;
    double          windoo_humidity     = 0;
    double          windoo_pressure     = 0;
    int             weather             = 0;
    std::string     phone_model;
    std::string     time_sent;
};

#endif
