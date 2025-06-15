#include <iostream>
#include <ctime>
#include <sstream>

#include "Capture.h"

int main() {
    std::time_t r = std::time(0);
    std::stringstream ss;
    ss << "./captures/" << r << ".mp4";
    Capture capture(ss.str());
    capture.startScreenRecord();
    std::cout << "Recording... press Enter to stop." << std::endl;
    std::cin.get();
    capture.endScreenRecord();
    return 0;
}
