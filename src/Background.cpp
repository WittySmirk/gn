#include "Background.h"

Background::Background(Settings* _settings, Capture* _capture, Gui* _gui): settings(_settings), capture(_capture), gui(_gui) {
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = (LPCSTR)"gn";
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
        std::cerr << "Failed to register capture hotkey" << std::endl;
    }

    if(!RegisterHotKey(hwnd, OPEN_GUI_ID, MOD_SHIFT, VK_F3)) {
        std::cerr << "Failed to register gui hotkey" << std::endl;
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
    switch(uMsg) {
        case WM_HOTKEY:
            if(wParam == START_STOP_ID) {
                if (!recording) {
                    recording = true;
                    std::time_t r = std::time(0);
                    std::stringstream ss;
                    ss << settings->outputFolder << r << ".mp4";
                    std::cout << ss.str() << std::endl;
                    capture->startScreenRecord(ss.str(), settings);
                    /*
                   
                    appThread = std::thread([this](){
                        std::time_t r = std::time(0);
                        std::stringstream ss;
                        ss << settings->outputFolder << r << ".mp4";
                        std::cout << ss.str() << std::endl;
                        capture->startScreenRecord(ss.str(), settings);
                    });
                    */
                } else {
                    capture->endScreenRecord();
                    recording = false;
                    //if(appThread.joinable()) appThread.join();
                }
                return 0;
            }
            if(wParam == OPEN_GUI_ID) {
                if(!guiOpen) {
                    guiOpen = true;
                    gui->spawn(settings);
                } else {
                    guiOpen = false;
                    gui->kill();
                }
            }            
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}