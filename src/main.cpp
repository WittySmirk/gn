#include <iostream>
#include <ctime>
#include <sstream>
#include <windows.h>

#include "Capture.h"
#include "Settings.h"
#include "Background.h"
#include "Gui.h"

// TODO: if want to make cross platform, probably sub out for SDL_main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Capture capture;

    Settings settings;
   
    Background background(&settings, &capture, settings.readSettings());
    background.listenForHotkey();

    return 0;
}
