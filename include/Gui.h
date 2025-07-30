#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>

#include "Settings.h"
#include "Capture.h"
#include "Element.h"
#include "MultiSelect.h"
#include "Text.h"
#include "Editor.h"

enum State {
    SETTINGS,
    SETUPSTAGE1,
    SETUPSTAGE2,
    SETUPSTAGE3,
    EDITINGSTAGE1,
    EDITINGSTAGE2
};

class Gui {
    public:
        Gui(Settings* _settings, std::function<void()> _callback, SDL_Surface* _icon, State _state);
        virtual ~Gui();
        void kill();
    private:
        std::function<void()> callback;
        static void folderCallback(void* userData, const char* const* files, int filter);
        static void fileCallback(void* userData, const char* const* files, int filter);
        void openWindow();
        void pickRightSetup();

        void createSetup();
        void createSettings();

        SDL_Surface* icon;

        SDL_Window* window;
        bool launchedFromBg = false;
        bool quit = false;

        State state;

        Editor* editor;
        std::string editFile;

        SettingsE currentSetting;

        TTF_Font* font;
        SDL_Renderer* renderer;

        Settings* settings = nullptr;

        std::vector<Element*> pipe;
};

#endif