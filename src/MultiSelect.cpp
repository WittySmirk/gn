#include "MultiSelect.h"

MultiSelect::MultiSelect(SDL_Renderer* _renderer, TTF_Font* _font, 
    std::string _main, std::vector<MultiItem> _items, int _x, int _y) 
    : Text(_renderer, _font, _main, _x, _y, 32, FOREGROUND_WHITE), font(_font) {
    Text::setBackColor(HIGHLIGHT_BLUE);
    Text::setButton([this](){
        toggled = !toggled;
        toggle();
    });

    SDL_FRect* lastRect = Text::getRect();

    for(MultiItem& i : _items) {
        std::cout << i.s << std::endl;
        Text* t = new Text(renderer, font, i.s.c_str(), lastRect->x, lastRect->y + lastRect->h, lastRect->w, lastRect->h, 32, FOREGROUND_WHITE);
       
        lastRect = t->getRect();
       
        t->setBackColor(HIGHLIGHT_BLUE);
        t->setButton([this, i](){
            SDL_Surface* surf = TTF_RenderText_Blended(font, i.s.c_str(), 0, FOREGROUND_WHITE);
            SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_DestroySurface(surf);
            setTexture(text);

            toggled = !toggled;
            toggle();
            i.f();
        });
        t->setRendered(false);
        children.push_back(t);
    }
}

MultiSelect::~MultiSelect() {
    for(Text* t : children) {
        delete t;
    }
}

void MultiSelect::checkMouse(SDL_MouseButtonEvent* _lastMouse) {
    Text::checkMouse(_lastMouse);
    for(Text* t : children) {
        t->checkMouse(_lastMouse);
    }
}

void MultiSelect::draw() {
    Text::draw();

    if(toggled) {
        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
        SDL_FRect nrect = {getRect()->x, getRect()->y + getRect()->h, getRect()->w, getRect()->h * children.size()};
        SDL_RenderFillRect(renderer, &nrect);
        for(Text* t : children) {
            t->draw();
        }
    }
}

void MultiSelect::toggle() {
    for(Text* t : children) {
        t->setRendered(toggled);
    }
}