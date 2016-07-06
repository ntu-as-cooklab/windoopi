#include <stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <string>
using std::cout;
using std::stringstream;
using std::string;

#include "WPiCURL.hpp"

void WPiCURL::init()
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://mospc.cook.as.ntu.edu.tw/post.php");
}

WPiCURL::~WPiCURL()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void WPiCURL::post()
{
    string poststring;
    stringstream poststream;
    poststream
        << "user_id="
        << user_id
        << "&windoo_id="
        << windoo_id
        << "&time_start="
        << time_start
        << "&time_finish="
        << time_finish
        << "&location_latitude="
        << location_latitude
        << "&location_longitude="
        << location_longitude
        << "&location_altitude="
        << location_altitude
        << "&location_heading="
        << location_heading
        << "&location_time="
        << location_time
        << "&windoo_wind="
        << windoo_wind
        << "&windoo_temperature="
        << windoo_temperature
        << "&windoo_humidity="
        << windoo_humidity
        << "&windoo_pressure="
        << windoo_pressure
        << "&weather="
        << weather
        << "&phone_model="
        << phone_model
        << "&time_sent="
        << time_sent;
    poststring = poststream.str();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, poststring.c_str());
        if((res = curl_easy_perform(curl)) != CURLE_OK)
            printf("POST failed: %s\n", curl_easy_strerror(res));
    }
}
