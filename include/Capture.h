#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>

#include "Settings.h"

class Capture {
    public:
        Capture();
        void startScreenRecord(std::string filename, Settings* settings);
        void endScreenRecord();
    private:
        PROCESS_INFORMATION ffmpegProcess = {0};
        HANDLE hStdInRead = NULL;
        HANDLE hStdInWrite = NULL;
};

#endif