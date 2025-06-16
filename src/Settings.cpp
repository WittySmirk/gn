#include "Settings.h"

Settings::Settings() {}

bool Settings::readSettings() {
    std::ifstream in("Settings");
    if(!in) {
        return false;
    }
    std::string line;
    
    int category = -1;
    int type = 0;

    while(std::getline(in, line)) {
        if(line.empty())
            continue;
        if(line.at(0) == '[') {
            category++;
            type = 0;
            continue;
        }
        std::stringstream afterS;
        bool before = true;
        for (int i = 0; i < line.length(); i++) {
            if (line.at(i) == '=') {
                before = false;
                continue;
            }
            if(!before)
                afterS << line.at(i);
        }
        
        if(category == 0) {
            switch(type) {
                case 0: afterS >> nvidia; break;
                case 1: afterS >> amd; break;
                case 2: steroDevice = afterS.str(); break;
                case 3: mic = afterS.str(); break;
            }
        } else {
            switch(type) {
                case 0: afterS >> fps; break;
                case 1: outputFolder = afterS.str(); break;
            }
        }

        type++;
    }
    in.close();
    return true;
}

void Settings::createSettings() {
    std::cout << "TODO: Create Settings" << std::endl;
}