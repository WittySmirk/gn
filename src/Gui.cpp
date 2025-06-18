#include "Gui.h"

Gui::Gui() {}

void Gui::openWindow() {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to init SDL video" << std::endl;
        exit(1);
    }
    // TODO: figure out flag for focusing window when opened
    window = SDL_CreateWindow("gn", 800, 600, SDL_WINDOW_INPUT_FOCUS);
    if(window == nullptr) {
        std::cerr << "Failed to init SDL window" << std::endl;
        exit(1);
    }
    SDL_ShowOpenFolderDialog(folderCallback, this, window, nullptr, false);
    // TODO: add while loop for handling input
}

void Gui::folderCallback(void* userData, const char* const* files, int filter) {
    // TODO: work on folder selection callback
}

void Gui::closeWindow() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}