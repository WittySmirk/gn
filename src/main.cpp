#include <iostream>
#include <ctime>
#include <sstream>

#include "Capture.h"
#include "Settings.h"
#include "Background.h"
#include "Gui.h"

int main() {
    Gui gui;
    Capture capture;

    Settings settings;
    if(!settings.readSettings()) {
        gui.spawn(&settings, true);
        return 0;
    }

    Background background(&settings, &capture, &gui);
    background.listenForHotkey();

    return 0;
}
