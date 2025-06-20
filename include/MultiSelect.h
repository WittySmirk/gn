#ifndef MULTI_SELECT_H
#define MULTI_SELECT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <vector>
#include <string>
#include <functional>

#include "Element.h"
#include "Gui.h"

struct MultiItem {
    std::string s;
    std::function<void()> f;
};

class MultiSelect: public Element {
    public:
        MultiSelect(SDL_Renderer* _renderer, TTF_Font* _font, std::string _main, std::vector<MultiItem> _items, int _x, int _y);
        ~MultiSelect();
        void checkMouse(SDL_MouseButtonEvent* _lastMouse);
        void draw();
        void free();
    private:
        void toggle();
        std::vector<Element*> children;
        bool toggled = false;
};

#endif