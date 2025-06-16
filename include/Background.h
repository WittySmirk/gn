#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <windows.h>
#include <iostream>
#include <thread>

#include "Capture.h"

#define START_STOP_ID 1

class Background {
    public:
        Background(Capture* _capture);
        ~Background();
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
        LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        void listenForHotkey();
    private:
        HWND hwnd;
        WNDCLASS wc = {};
        Capture* capture;
};


#endif