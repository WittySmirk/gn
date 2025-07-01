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
    // MIC,
    FPS,
    OUTPUTFOLDER,
    CLIPSFOLDER,
};

struct Settings {
    Settings();
    
    bool readSettings();
    bool writeSettingsFile();
    void detectGPU(); // Not sure if this is needed

    // Hardware
    bool nvidia = false;
    bool amd = false;
    //std::string mic = "";

    // General
    int fps = NULL;
    std::string outputFolder = "";
    std::string clipsFolder = ""; 
};

#endif