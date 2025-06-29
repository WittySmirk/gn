#include "Element.h"

Element::Element(SDL_Renderer* _renderer): renderer(_renderer) {}

Element::~Element() {
    SDL_DestroyTexture(texture);
    delete rect;
}

void Element::checkMouse(SDL_MouseButtonEvent* _lastMouse) {
    if (clickable && rendered) {
        if(_lastMouse->x >= rect->x && _lastMouse->x <= rect->x + rect->w
            && _lastMouse->y >= rect->y && _lastMouse->y <= rect->y + rect->h)
                clickCallback();

    }
}

void Element::draw() {
    if (rendered) {
        if(texture != nullptr) {
                if (button) {
                    SDL_SetRenderDrawColor(renderer, backColor.r, backColor.g, backColor.b, backColor.a);
                    SDL_RenderRect(renderer, rect);
                }
                SDL_RenderTexture(renderer, texture, nullptr, rect);
        }
        if (rect != nullptr && colorSet) {
            SDL_SetRenderDrawColor(renderer, backColor.r, backColor.g, backColor.b, backColor.a);
            SDL_RenderFillRect(renderer, rect);
        }
    }
}

void Element::setRect(SDL_FRect* _rect) {
    if(rect != nullptr) {
        delete rect;
    }
    rect = _rect;
}
void Element::setTexture(SDL_Texture* _texture) {
    if(texture != nullptr) {
        SDL_DestroyTexture(texture);
    }
    texture = _texture;
}
void Element::setBackColor(SDL_Color _backColor) {
    colorSet = true;
    backColor = _backColor;
}
void Element::setButton(std::function<void()> _callback) {
    button = true;
    clickable = true;
    clickCallback = _callback;
}
void Element::setRendered(bool _rendered) {
    rendered = _rendered;
}

SDL_FRect* Element::getRect() {
    return rect;
}

SDL_Renderer* Element::getRenderer() {
    return renderer;
}