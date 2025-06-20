#ifndef TEXT_H
#define TEXT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Element.h"

class Text : public Element {
    public:
        // Default will go in center
        Text(SDL_Renderer* _renderer, TTF_Font* _font, std::string _text, float _of, int _size, SDL_Color _color);
        Text(SDL_Renderer* _renderer, TTF_Font* _font, std::string _text, int _x, int _y, int _size, SDL_Color _color);
        Text(SDL_Renderer* _renderer, TTF_Font* _font, std::string _text, int _x, int _y, int _w, int _h, int _size, SDL_Color _color);
};

#endif