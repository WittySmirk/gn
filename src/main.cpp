#include <iostream>
#include <ctime>
#include <sstream>

#include "Capture.h"
#include "Settings.h"
#include "Background.h"
#include "Gui.h"

int main() {
    Settings settings;
    if(!settings.readSettings()) {
        // TODO: initial setup
        settings.createSettings();
    }
    std::time_t r = std::time(0);
    std::stringstream ss;
    ss << settings.outputFolder << r << ".mp4";

    Gui gui;

    Capture capture(ss.str(), &settings);

    Background background(&capture, &gui);
    background.listenForHotkey();

    return 0;
}
