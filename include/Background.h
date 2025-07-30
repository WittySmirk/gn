#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <thread>
#include <ctime>

#include "Settings.h"
#include "Capture.h"
#include "Gui.h"

#define START_STOP_ID 1
#define OPEN_GUI_ID 2

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_EDIT 1002

class Background {
    public:
        Background(Settings* _settings, Capture* _capture);
        ~Background();
        void listenForHotkey();
    private:
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
        LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        // std::thread appThread;
        bool recording = false;
        bool guiOpen = false;

        HWND hwnd;
        WNDCLASS wc = {};

        NOTIFYICONDATA nid = {};
        HMENU hTrayMenu = nullptr;

        Capture* capture;
        Gui* gui = nullptr;
        Settings* settings;
};


#endif