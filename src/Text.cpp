#include "Text.h"

Text::Text(SDL_Renderer* _renderer, TTF_Font* _font, std::string _text, float _off, int _size, SDL_Color _color)
: Element(_renderer) {
    TTF_SetFontSize(_font, _size);
    
    SDL_Surface* surf = TTF_RenderText_Blended(_font, _text.c_str(), 0, _color);
    SDL_Texture* text = SDL_CreateTextureFromSurface(_renderer, surf);
    int textW = surf->w;
    int textH = surf->h;
    SDL_DestroySurface(surf);

    SDL_FRect* rect = new SDL_FRect{(float)((SCREEN_W / 2) - (textW / 2)), ((SCREEN_H / 2) - (textH / 2)) * _off, (float)textW, (float)textH};

    Element::setTexture(text);
    Element::setRect(rect);
}


Text::Text(SDL_Renderer* _renderer, TTF_Font* _font, std::string _text, int _x, int _y, int _size, SDL_Color _color)
: Element(_renderer) {
    TTF_SetFontSize(_font, _size);
    
    SDL_Surface* surf = TTF_RenderText_Blended(_font, _text.c_str(), 0, _color);
    SDL_Texture* text = SDL_CreateTextureFromSurface(_renderer, surf);
    int textW = surf->w;
    int textH = surf->h;
    SDL_DestroySurface(surf);

    SDL_FRect* rect = new SDL_FRect{(float)_x, (float)_y, (float)textW, (float)textH};

    Element::setTexture(text);
    Element::setRect(rect);
}

Text::Text(SDL_Renderer* _renderer, TTF_Font* _font, std::string _text, int _x, int _y, int _w, int _h, int _size, SDL_Color _color)
: Element(_renderer) {
    TTF_SetFontSize(_font, _size);
    
    int textW = _w;
    int textH = _h;

    SDL_Surface* surf = TTF_RenderText_Blended(_font, _text.c_str(), 0, _color);
    SDL_Texture* text = SDL_CreateTextureFromSurface(_renderer, surf);
    
    SDL_DestroySurface(surf);

    SDL_FRect* rect = new SDL_FRect{(float)_x, (float)_y, (float)textW, (float)textH};

    Element::setTexture(text);
    Element::setRect(rect);
}