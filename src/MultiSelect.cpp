#include "MultiSelect.h"

MultiSelect::MultiSelect(SDL_Renderer* _renderer, TTF_Font* _font, 
    std::string _main, std::vector<MultiItem> _items, int _x, int _y) 
    : Element(_renderer) {
    SDL_Surface* s1 = TTF_RenderText_Blended(_font, _main.c_str(), 0, FOREGROUND_WHITE);
    SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, s1);
    int textW = s1->w;
    int textH = s1->h;
    SDL_DestroySurface(s1);

    SDL_FRect* r1 = new SDL_FRect{(float)_x, (float)_y, (float)textW, (float)textH};

    Element::setTexture(t1);
    Element::setRect(r1);
    Element::setBackColor(HIGHLIGHT_BLUE);
    Element::setButton([this](){
        toggled = !toggled;
        toggle();
    });

    SDL_FRect* lastRect = r1;

    for(MultiItem& i : _items) {
        std::cout << i.s << std::endl;
        SDL_Surface* surf = TTF_RenderText_Blended(_font, i.s.c_str(), 0, FOREGROUND_WHITE);
        SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);

        SDL_FRect* rect = new SDL_FRect((float)lastRect->x, (float)(lastRect->y + lastRect->h), lastRect->w, lastRect->h);
        lastRect = rect;

        Element* e = new Element(_renderer);
        e->setTexture(text);
        e->setRect(rect);
        e->setBackColor(HIGHLIGHT_BLUE);
        e->setButton(i.f);
        e->setRendered(false);
        children.push_back(e);
    }
}

MultiSelect::~MultiSelect() {
    for(Element* e : children) {
        delete e;
    }
}

void MultiSelect::checkMouse(SDL_MouseButtonEvent* _lastMouse) {
    Element::checkMouse(_lastMouse);
    for(Element* e : children) {
        e->checkMouse(_lastMouse);
    }
}

void MultiSelect::draw() {
    Element::draw();

    for(Element* e : children) {
        e->draw();
    }
}

void MultiSelect::toggle() {
    for(Element* e : children) {
        e->setRendered(toggled);
    }
}