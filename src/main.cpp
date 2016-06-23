#include <stdlib.h>
#include <stdio.h>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;

    wpiEngine.init();
    //wpiEngine.printDevs();
    wpiEngine.record();
    Pa_Sleep(2000);
    wpiEngine.play();
    wpiEngine.terminate();

    return 0;
}
