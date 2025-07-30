#include "Background.h"

Background::Background(Settings* _settings, Capture* _capture, bool _notFirst): settings(_settings), capture(_capture) {
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

    HICON hIco = (HICON)LoadImage(NULL, "./assets/gn.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIco);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIco);

    hTrayMenu = CreatePopupMenu();
    AppendMenu(hTrayMenu, MF_STRING, ID_TRAY_EDIT, "Open Editor");
    AppendMenu(hTrayMenu, MF_STRING, ID_TRAY_EXIT, "Quit gn");

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIco;
    strcpy_s(nid.szTip, "gn");

    Shell_NotifyIcon(NIM_ADD, &nid);
    
    SetSurfaceFromIco(hIco);
    if(!_notFirst) {
        guiOpen = true;
        completingFirst = true;
        Gui* g = new Gui(settings, [&](){completingFirst = false;}, iconSurface, true);
    }
}

Background::~Background() {
    UnregisterHotKey(hwnd, START_STOP_ID);
    Shell_NotifyIcon(NIM_DELETE, &nid);
    DestroyMenu(hTrayMenu);
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
        case WM_TRAYICON:
            if(lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);

                TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case ID_TRAY_EDIT:
                    if(!guiOpen) {
                        guiOpen = true;
                        gui = new Gui(settings, [&](){guiOpen = false;}, iconSurface, false);
                    }
                    break;
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            break;
        case WM_HOTKEY:
            if(wParam == START_STOP_ID) {
                if(!completingFirst) {
                    if (!recording) {
                        recording = true;
                        std::time_t r = std::time(0);
                        std::stringstream ss;
                        ss << settings->outputFolder << r << ".mp4";
                        std::cout << ss.str() << std::endl;
                        capture->startScreenRecord(ss.str(), settings);
                    } else {
                        capture->endScreenRecord();
                        recording = false;
                    }
                }
                return 0;
            }
            if(wParam == OPEN_GUI_ID) {
                if(!guiOpen) {
                    guiOpen = true;
                    gui = new Gui(settings, [&](){guiOpen = false;}, iconSurface, false);
                } 
            }            
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Background::SetSurfaceFromIco(HICON _hIco) {
    ICONINFO iconInfo;
    if (!GetIconInfo(_hIco, &iconInfo)) return;

    BITMAP bmp;
    if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp)) {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return;
    }

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    HDC hdc = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, iconInfo.hbmColor);

    void* pixels = malloc(width * height * 4);
    if (!pixels) {
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // negative to flip top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    if (!GetDIBits(memDC, iconInfo.hbmColor, 0, height, pixels, &bmi, DIB_RGB_COLORS)) {
        free(pixels);
        pixels = nullptr;
    }

    SelectObject(memDC, oldBitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    if (!pixels) return;

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        width,
        height,
        SDL_PIXELFORMAT_ARGB8888,
        pixels,
        width * 4  // pitch (bytes per row)
    );

    if(!surface) {
        free(pixels);
    }

    iconSurface = surface;
}