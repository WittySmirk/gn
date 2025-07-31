#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>

#include <obs.h>

#include "Settings.h"

class Capture {
    public:
        Capture();
        ~Capture();
        void startScreenRecord(std::string _filename, Settings* _settings);
        void endScreenRecord();
    private:
        obs_output_t* output;
        obs_source_t* capture;
        obs_encoder_t* vEnc;
        obs_encoder_t* aEnc;
};

#endif