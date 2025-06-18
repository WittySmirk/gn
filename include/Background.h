#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <windows.h>
#include <iostream>
#include <thread>

#include "Capture.h"
#include "Gui.h"

#define START_STOP_ID 1
#define OPEN_GUI_ID 2

class Background {
    public:
        Background(Capture* _capture, Gui* _gui);
        ~Background();
        void listenForHotkey();
    private:
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
        LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        HWND hwnd;
        WNDCLASS wc = {};
        Capture* capture;
        bool guiOpen = false;
        Gui* gui;
};


#endif