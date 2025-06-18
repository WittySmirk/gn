#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <iostream>

class Gui {
    public:
        Gui();
        void openWindow();
        void closeWindow();
    private:
        static void folderCallback(void* userData, const char* const* files, int filter);
        SDL_Window* window;
};

#endif