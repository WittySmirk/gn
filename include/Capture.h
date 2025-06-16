#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>

#include "Settings.h"

class Capture {
    public:
        Capture(std::string _filename, Settings* _settings);
        void startScreenRecord();
        void endScreenRecord();
    private:
        std::string fileName;
        PROCESS_INFORMATION ffmpegProcess = {0};
        HANDLE hStdInRead = NULL;
        HANDLE hStdInWrite = NULL;
        Settings* settings;
};

#endif