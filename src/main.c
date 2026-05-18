#include <stdio.h>
#include <stdlib.h>

#include "ppu.h"

int main(int argc, char *argv[])
{
    Tigr *screen = tigrWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gameboy Emulator", 0);
    while (!tigrClosed(screen))
    {
        tigrClear(screen, tigrRGB(0xff, 0xff, 0xff));
        tigrPrint(screen, tfont, 40, 50, tigrRGB(0xf0, 0x00, 0x00), "Hello.");
        tigrPlot(screen, 0, 0, tigrRGB(0x00, 0x00, 0xf0));
        tigrUpdate(screen);
    }
    tigrFree(screen);

    return 0;
}