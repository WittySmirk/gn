#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <cstdio>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

enum SettingsE {
    NVIDIA,
    AMD,
    STEREODEVICE,
    MIC,
    FPS,
    OUTPUTFOLDER
};

struct Settings {
    Settings();
    
    bool readSettings();
    void writeSettingsFile();
    void detectGPU();
    std::vector<std::string> findAudioDevices();

    // Hardware
    bool nvidia = false;
    bool amd = false;
    std::string steroDevice;
    std::string mic;

    // General
    int fps;
    std::string outputFolder;
};

#endif