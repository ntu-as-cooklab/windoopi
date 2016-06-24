#include <stdlib.h>
#include <stdio.h>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;

    wpiEngine.init();
    //wpiEngine.printDevs();
    //wpiEngine.record();
    //Pa_Sleep(3000);
    //wpiEngine.play();
    wpiEngine.initSineWavetable();
    wpiEngine.playWave();
    wpiEngine.terminate();

    return wpiEngine.err;
}
