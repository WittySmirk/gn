#include <iostream>
#include <ctime>
#include <sstream>

#include "Capture.h"
#include "Settings.h"
#include "Background.h"
#include "Gui.h"

int main() {
    Capture capture;

    Settings settings;
    /*
        if(!settings.readSettings()) {
            Gui* g = new Gui(&settings, [&](){}, true);
        }
    */
    Background background(&settings, &capture, settings.readSettings());
    background.listenForHotkey();

    return 0;
}
