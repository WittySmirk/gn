#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

struct Settings {
    Settings();
    
    bool readSettings();
    void createSettings();
    // TODO: template function for getting setting
    // Hardware
    bool nvidia;
    bool amd;
    std::string steroDevice;
    std::string mic;

    // General
    int fps;
    std::string outputFolder;
};

#endif