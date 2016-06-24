#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;

    wpiEngine.init();
    //wpiEngine.printDevs();
    std::string filename = "record.raw";
    wpiEngine.filename = filename.c_str();
    wpiEngine.record();
    Pa_Sleep(2000);
    wpiEngine.play();
    //wpiEngine.genSineWavetable();
    //wpiEngine.playWave();
    wpiEngine.terminate();

    return wpiEngine.err;
}
