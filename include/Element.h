#ifndef ELEMENT_H
#define ELEMENT_H

#include <SDL3/SDL.h>
#include <iostream>
#include <functional>

#define SCREEN_W 1280
#define SCREEN_H 720

// TODO: Work on color schemeing
#define BACKGROUND SDL_Color {0, 0, 0, 0}
#define FOREGROUND SDL_Color {255, 255, 255, 0}
#define HIGHLIGHT SDL_Color {0, 174, 255, 0}

class Element {
    public:
        Element(SDL_Renderer* _renderer);
        virtual ~Element();
        virtual void checkMouse(SDL_MouseButtonEvent* _lastMouse);
        virtual void draw();

        void setRect(SDL_FRect* _rect);
        void setTexture(SDL_Texture* _texture);
        void setBorderColor(SDL_Color _borderColor);
        void setBackColor(SDL_Color _borderColor);
        void setButton(std::function<void()>);
        void setRendered(bool _rendered);
        
        virtual bool hasOverlay() {return false;} // TODO: rework to not need this

        // Input viruals
        virtual bool getFocused() {return false;}
        virtual void collectText(std::string _text) {}
        virtual void deleteText() {}
        virtual void clearText() {}
        virtual std::string getText() {return "";}

        bool getRendered() { return rendered;}
        SDL_FRect* getRect();
        SDL_Renderer* getRenderer();
    protected:
        SDL_Renderer* renderer = nullptr;
    private:
        bool clickable = false;
        bool button = false;
        bool rendered = true;
        bool borderFilled = false;
        bool backFilled = false;
        SDL_FRect* rect = nullptr;
        SDL_Texture* texture = nullptr;
        SDL_Color borderColor;
        SDL_Color backColor;
        std::function<void()> clickCallback;
};

#endif