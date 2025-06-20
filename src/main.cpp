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
        // TODO: initial setup
        gui.spawn(&settings, true);
    }
    std::time_t r = std::time(0);
    std::stringstream ss;
    ss << settings.outputFolder << r << ".mp4";

    Background background(&settings, &capture, &gui);
    background.listenForHotkey();

    return 0;
}
