#include "Background.h"

Background::Background(Capture* _capture): capture(_capture) {
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = (LPCSTR)"GN";
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0, 
        wc.lpszClassName, 
        (LPCSTR)"Hidden", 
        0, 
        0, 
        0, 
        0, 
        0, 
        NULL, 
        NULL, 
        wc.hInstance, 
        this
    );
    if(!hwnd) {
        std::cerr << "Failed to create hidden window" << std::endl;
    }
}

Background::~Background() {
    UnregisterHotKey(hwnd, START_STOP_ID);
}

void Background::listenForHotkey() {
    if(!RegisterHotKey(hwnd, START_STOP_ID, MOD_SHIFT, VK_F4)) {
        std::cerr << "Failed to register hotkey" << std::endl;
    }

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK Background::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Background* self = nullptr;
    if(uMsg == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        self = (Background*)pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self); 
    } else {
        self = (Background*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (self) {
        return self->handleMessage(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT Background::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::thread appThread;
    static bool recording = false;

    switch(uMsg) {
        case WM_HOTKEY:
            if(wParam == START_STOP_ID) {
                if (!recording) {
                    recording = true;
                    appThread = std::thread([this](){
                        capture->startScreenRecord();
                    });
                } else {
                    capture->endScreenRecord();
                    recording = false;
                    if(appThread.joinable()) appThread.join();
                }
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}