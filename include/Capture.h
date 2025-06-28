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
        void startScreenRecord(std::string _filename, Settings* _settings);
        void endScreenRecord();
    private:
        obs_output_t* output;
        obs_source_t* capture;
        /*
        PROCESS_INFORMATION ffmpegProcess = {0};
        HANDLE hStdInRead = NULL;
        HANDLE hStdInWrite = NULL;
        */
};

#endif