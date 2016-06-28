#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;
    wpiEngine.init();
    //wpiEngine.printDevs();
    //std::string filename = "record.raw";
    //wpiEngine.filename = filename.c_str();
    wpiEngine.serialWrite();
    //wpiEngine.windoo();
    wpiEngine.terminate();
    return wpiEngine.err;
}
