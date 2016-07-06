#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "WpiEngine.hpp"

int main()
{
    WpiEngine wpiEngine;
    wpiEngine.init();
    wpiEngine.windoo();
    wpiEngine.terminate();
    return wpiEngine.err;
}
