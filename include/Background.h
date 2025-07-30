#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <thread>
#include <ctime>

#include <SDL3/SDL.h>

#include "Settings.h"
#include "Capture.h"
#include "Gui.h"

#define START_STOP_ID 1
#define OPEN_GUI_ID 2

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_EDIT 1002
#define ID_TRAY_SETTINGS 1003

class Background {
    public:
        Background(Settings* _settings, Capture* _capture, bool _notFirst);
        ~Background();
        void listenForHotkey();
    private:
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
        LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        // std::thread appThread;
        bool recording = false;
        bool guiOpen = false;
        bool completingFirst = false;

        HWND hwnd;
        WNDCLASS wc = {};

        NOTIFYICONDATA nid = {};
        HMENU hTrayMenu = nullptr;

        Capture* capture;
        Gui* gui = nullptr;
        Settings* settings;

        SDL_Surface* iconSurface = nullptr;
        void setSurfaceFromIco(HICON _hIco);
};

#endif