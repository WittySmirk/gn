#include "Input.h"

Input::Input(SDL_Renderer* _renderer, TTF_Font* _font, std::string _notion, float _off, int _size)
: Text(_renderer, _font, _notion, _off, _size, FOREGROUND), 
  font(_font), size(_size), off(_off), notion(_notion)  {
    setBackColor(BACKGROUND);
    setBorderColor(HIGHLIGHT);
}

void Input::draw() {
    Text::draw();
}

void Input::collectText(std::string _text) {
    text = text + _text;
    renderText(text);
}

void Input::deleteText() {
    if(text != "" && !text.empty() && text.size() != 1) {
        text.pop_back();
        renderText(text);
        return;
    }
    clearText();
}

void Input::clearText() {
    std::cout << "clear text" << std::endl;
    text = "";
    renderText(notion);
}

std::string Input::getText() {
    return text;
}

void Input::renderText(std::string _text) {
    TTF_SetFontSize(font, size);

    SDL_Surface* surf = TTF_RenderText_Blended(font, _text.c_str(), 0, FOREGROUND);
    SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surf);
    int textW = surf->w;
    int textH = surf->h;
    SDL_DestroySurface(surf);

    SDL_FRect* rect = new SDL_FRect{(float)((SCREEN_W / 2) - (textW / 2)), ((SCREEN_H / 2) - (textH / 2)) * off, (float)textW, (float)textH};

    Element::setRect(rect);
    Element::setTexture(text);
}

bool Input::getFocused() {
    return focused;
}

void Input::setFocused(bool _focused) {
    focused = _focused;
}