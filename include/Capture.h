#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include <windows.h>
#include <string>
#include <sstream>

class Capture {
    public:
        Capture(std::string filename);
        void startScreenRecord();
        void endScreenRecord();
    private:
        std::string outFilename;
        PROCESS_INFORMATION ffmpegProcess = {0};
        HANDLE hStdInRead = NULL;
        HANDLE hStdInWrite = NULL;
};

#endif