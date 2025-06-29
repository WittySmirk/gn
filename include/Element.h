#ifndef ELEMENT_H
#define ELEMENT_H

#include <SDL3/SDL.h>
#include <iostream>
#include <functional>

#define SCREEN_W 1280
#define SCREEN_H 720

// TODO: Work on color schemeing
#define BACKGROUND_COLOR SDL_Color {0, 0, 0, 0}
#define FOREGROUND_WHITE SDL_Color {255, 255, 255, 0}
#define HIGHLIGHT_BLUE SDL_Color {21, 237, 198, 0}
#define FOREGROUND_YELLOW SDL_Color {246, 236, 18, 0}

class Element {
    public:
        Element(SDL_Renderer* _renderer);
        virtual ~Element();
        virtual void checkMouse(SDL_MouseButtonEvent* _lastMouse);
        virtual void draw();

        void setRect(SDL_FRect* _rect);
        void setTexture(SDL_Texture* _texture);
        void setBackColor(SDL_Color _borderColor);
        void setButton(std::function<void()>);
        void setRendered(bool _rendered);
        
        virtual bool hasOverlay() {return false;}

        bool getRendered() { return rendered;}
        SDL_FRect* getRect();
        SDL_Renderer* getRenderer();
    protected:
        SDL_Renderer* renderer = nullptr;
    private:
        bool clickable = false;
        bool button = false;
        bool rendered = true;
        bool colorSet = false;
        SDL_FRect* rect = nullptr;
        SDL_Texture* texture = nullptr;
        SDL_Color backColor;
        std::function<void()> clickCallback;
};

#endif