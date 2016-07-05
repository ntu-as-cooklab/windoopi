#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;
    wpiEngine.init();
    //wpiEngine.printDevs();
    std::string filename = "/home/alarm/windoo.log";
    wpiEngine.filename = filename.c_str();
    wpiEngine.windoo();
    wpiEngine.terminate();
    return wpiEngine.err;
}
