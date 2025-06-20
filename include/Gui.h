#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <vector>

#include "Settings.h"
#include "Capture.h"
#include "Element.h"
#include "MultiSelect.h"
#include "Text.h"

#define BACKGROUND_COLOR SDL_Color {34, 32, 52, 0}
#define FOREGROUND_WHITE SDL_Color {226, 255, 211, 0}
#define HIGHLIGHT_BLUE SDL_Color {21, 237, 198, 0}
#define FOREGROUND_YELLOW SDL_Color {246, 236, 18, 0}

enum State {
    SETTINGS,
    SETUPSTAGE1,
    SETUPSTAGE2,
    SETUPSTAGE3,
    EDITING
};

class Gui {
    public:
        Gui();
        void spawn(Settings* _settings, bool _setup = false);
        void kill();
    private:
        static void folderCallback(void* userData, const char* const* files, int filter);
        void openWindow();
        void createSetup();
        void pickRightSetup();

        SDL_Window* window;
        bool launchedFromBg = false;
        bool quit = false;

        State state;

        SettingsE currentSetting;

        TTF_Font* font;
        SDL_Renderer* renderer;

        Settings* settings = nullptr;

        std::vector<Element*> pipe;
};

#endif