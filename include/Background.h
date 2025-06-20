#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <windows.h>
#include <iostream>
#include <thread>
#include <ctime>

#include "Settings.h"
#include "Capture.h"
#include "Gui.h"

#define START_STOP_ID 1
#define OPEN_GUI_ID 2

class Background {
    public:
        Background(Settings* _settings, Capture* _capture, Gui* _gui);
        ~Background();
        void listenForHotkey();
    private:
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
        LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        std::thread appThread;
        bool recording = false;
        bool guiOpen = false;

        HWND hwnd;
        WNDCLASS wc = {};
        Capture* capture;
        Gui* gui;
        Settings* settings;
};


#endif