#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;
    wpiEngine.init();
    //std::string filename = "record.raw";
    //wpiEngine.filename = filename.c_str();
    wpiEngine.windoo();
    wpiEngine.terminate();
    return wpiEngine.err;
}
