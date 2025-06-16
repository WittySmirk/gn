#include "Capture.h"

Capture::Capture(std::string _filename, Settings* _settings): fileName(_filename), settings(_settings) {}

void Capture::startScreenRecord() {
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0);
    SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = hStdInRead;
    si.hStdError = NULL;
    si.hStdOutput = NULL;

    std::stringstream ss;
 
    ss << "ffmpeg -f gdigrab -framerate " << settings->fps << " -i desktop " << "-f dshow -i audio=\"" << settings->steroDevice << "\" ";
    if (settings->nvidia)
        ss << "-c:v h264_nvenc -qp 0 ";
    ss << fileName;

    std::string cmd = ss.str();
    

    if(!CreateProcess(NULL, (LPSTR)cmd.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &ffmpegProcess)) {
        std::cerr << "Failed to create process" << std::endl;
        return;
    } else {
        std::cout << "started screen record" << std::endl;
    }
}

void Capture::endScreenRecord() {
    DWORD bytesWritten;
    if(WriteFile(hStdInWrite, "q\n", 2, &bytesWritten, NULL)) {
        std::cout << "sent q" << std::endl; 
        CloseHandle(hStdInWrite); // close our end
        WaitForSingleObject(ffmpegProcess.hProcess, INFINITE); // wait for FFmpeg to finish
        CloseHandle(ffmpegProcess.hProcess);
        CloseHandle(ffmpegProcess.hThread);
    }
}