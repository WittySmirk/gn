#include <iostream>
#include <ctime>
#include <sstream>

#include "Capture.h"
#include "Settings.h"

int main() {
    Settings settings;
    if(!settings.readSettings()) {
        settings.createSettings();
    }
    std::time_t r = std::time(0);
    std::stringstream ss;
    ss << settings.outputFolder << r << ".mp4";

    Capture capture(ss.str(), &settings);
    capture.startScreenRecord();
    
    std::cout << "Recording... press Enter to stop." << std::endl;
    std::cin.get();
    capture.endScreenRecord();
    
    return 0;
}
