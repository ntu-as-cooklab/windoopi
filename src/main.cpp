#include <stdlib.h>
#include <stdio.h>

#include "wpiengine.hpp"

int main()
{
    WpiEngine wpiEngine;

    wpiEngine.init();
    //wpiEngine.printDevs();
    wpiEngine.record();
    wpiEngine.terminate();

    return 0;
}
