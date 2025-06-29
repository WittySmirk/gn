#ifndef INPUT_H
#define INPUT_H

#include <string>

#include "Element.h"
#include "Text.h"

class Input : public Text {
    public:
        Input(SDL_Renderer* _renderer, TTF_Font* _font, std::string _notion, float _off, int _size);
        void draw();
        void collectText(std::string _text);
        
        bool getFocused();
        void setFocused(bool _focused);
        void deleteText();
        void clearText();
        std::string getText();
    private:
        void renderText(std::string _text);

        TTF_Font* font;
        int size;
        float off;
        bool focused = false;
        std::string notion;
        std::string text = "";
};

#endif