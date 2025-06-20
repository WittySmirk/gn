#include "Gui.h"

Gui::Gui() {}

void Gui::spawn(Settings* _settings, bool _setup) {
    quit = false;
    settings = _settings;
    // TODO: better state management
    if (_setup) {
        state = State::SETUPSTAGE1;
    } else {
        state = State::EDITING;
    }

    openWindow();
}

void Gui::pickRightSetup() {
 switch(state) {
        case State::SETUPSTAGE1:
        case State::SETUPSTAGE2:
        case State::SETUPSTAGE3:
            createSetup();
        break;
    }

}

void Gui::openWindow() {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to init SDL video" << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow("gn", SCREEN_W, SCREEN_H, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS);
    if(window == nullptr) {
        std::cerr << "Failed to init SDL window" << std::endl;
        exit(1);
    }

    if(!TTF_Init()) {
        std::cerr << "Failed to init SDL_ttf" << std::endl;
        exit(1);
    }
    font = TTF_OpenFont("./assets/FunnelSans.TTF", 48);
    if(font == nullptr) {
        std::cerr << "Faile to load font" << std::endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    pickRightSetup();

    SDL_Event event;
    State lastKnownState = state;

    while(!quit) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if(event.key.key == SDLK_ESCAPE) {
                    quit = true;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                for (Element* e : pipe) {
                    e->checkMouse(&event.button);
                }
            }
        }

        if (state != lastKnownState) {
            for (Element* e : pipe) {
                delete e;
            }
            pipe.clear();

            pickRightSetup();
            lastKnownState = state;
            continue;
        }

        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
        SDL_RenderClear(renderer);

        for (Element* e : pipe) {
            e->draw();
        }

        SDL_RenderPresent(renderer);
    }

    settings = nullptr;
    for (Element* e : pipe) {
        delete e;
    }
    pipe.clear();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void Gui::createSetup() {
    switch(state) {
        case State::SETUPSTAGE1: {
            SDL_Surface* s1 = TTF_RenderText_Blended(font, "First Time?", 0, FOREGROUND_WHITE);
            SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, s1);
            int textW = s1->w;
            int textH = s1->h;
            SDL_DestroySurface(s1);
            SDL_FRect* r1 = new SDL_FRect{(float)((SCREEN_W / 2) - (textW / 2)), (float)((SCREEN_H / 2) - (textH / 2)) * 0.4f, (float)textW, (float)textH};

            Element* e= new  Element(renderer);
            e->setTexture(t1);
            e->setRect(r1);
            pipe.push_back(e);

            TTF_SetFontSize(font, 34);
            SDL_Surface* s2 = TTF_RenderText_Blended(font, "Setup", 0, FOREGROUND_WHITE);
            SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, s2);
            textW = s2->w;
            textH = s2->h;
            SDL_FRect* r2 = new SDL_FRect{(float)((SCREEN_W / 2) - (textW / 2)), (float)((SCREEN_H / 2) - (textH / 2)) * 0.8f, (float)textW, (float)textH};
            
            Element* e2 = new Element(renderer);
            e2->setTexture(t2);
            e2->setRect(r2);
            e2->setBackColor(HIGHLIGHT_BLUE);
            e2->setButton([this](){
                state = State::SETUPSTAGE2;
            });
            pipe.push_back(e2);

            break;
        }
        case State::SETUPSTAGE2:{
            settings->detectGPU();

            TTF_SetFontSize(font, 40);
            SDL_Surface* s1 = TTF_RenderText_Blended(font, "Pick Capture Location", 0, FOREGROUND_WHITE);
            SDL_Texture* t1 = SDL_CreateTextureFromSurface(renderer, s1);
            int textW = s1->w;
            int textH = s1->h;
            SDL_DestroySurface(s1);
            SDL_FRect* r1 = new SDL_FRect{100.0f, 100.0f, (float)textW, (float)textH};

            Element* e1 = new Element(renderer);
            e1->setTexture(t1);
            e1->setRect(r1);
            e1->setButton([this](){
                currentSetting = SettingsE::OUTPUTFOLDER;
                SDL_ShowOpenFolderDialog(folderCallback, this, window, nullptr, false);
            });
            e1->setBackColor(HIGHLIGHT_BLUE);
            pipe.push_back(e1);

            MultiSelect* m = new MultiSelect(renderer, font, "Select fps", std::vector<MultiItem>{
                MultiItem{"30 fps", [this](){settings->fps = 30;}},
                MultiItem{"60 fps", [this](){settings->fps = 60;}},
                MultiItem{"120 fps", [this](){settings->fps = 120;}}
            }, 
                200, 200);
            pipe.push_back(m);

            std::vector<MultiItem> audioItems;
            std::vector<std::string> devices = settings->findAudioDevices();
            for(std::string& s : devices) {
                MultiItem i = {s, [s, this](){
                    settings->steroDevice = s;
                    std::cout << settings->steroDevice << std::endl;
                }};
                audioItems.push_back(i);
            }

            MultiSelect* m2 = new MultiSelect(renderer, font, "Select Stereo Device", audioItems, 300, 300);
            pipe.push_back(m2);
            
            // Have to change the lambdas for setting mic
            audioItems.clear();
            for(std::string& s : devices) {
                MultiItem i = {s, [s, this](){
                    settings->mic = s;
                    std::cout << settings->mic << std::endl;
                }};
                audioItems.push_back(i);
            }

            MultiSelect* m3 = new MultiSelect(renderer, font, "Select Mic", audioItems, 400, 400);
            pipe.push_back(m3);

            std::string gpu;
            if (settings->amd) {
                gpu = "Detected AMD gpu";
            } else if (settings->nvidia) {
                gpu = "Detected NVIDIA gpu";
            } else {
                gpu = "No gpu detected";
            }

            SDL_Surface* s2 = TTF_RenderText_Blended(font, gpu.c_str(), 0, FOREGROUND_WHITE);
            SDL_Texture* t2 = SDL_CreateTextureFromSurface(renderer, s2);
            textW = s2->w;
            textH = s2->h;
            SDL_DestroySurface(s2);

            SDL_FRect* r2 = new SDL_FRect{600.0f, 600.0f, (float)textW, (float)textH};

            Element* e2 = new Element(renderer);
            e2->setTexture(t2);
            e2->setRect(r2);
            pipe.push_back(e2);


            SDL_Surface* s3 = TTF_RenderText_Blended(font, "Finished", 0, FOREGROUND_WHITE);
            SDL_Texture* t3 = SDL_CreateTextureFromSurface(renderer, s3);
            textW = s3->w;
            textH = s3->h;
            SDL_DestroySurface(s3);

            SDL_FRect* r3 = new SDL_FRect{700.0f, 500.0f, (float)textW, (float)textH};
            Element* e3 = new Element(renderer);
            e3->setTexture(t3);
            e3->setRect(r3);
            e3->setBackColor(HIGHLIGHT_BLUE);
            e3->setButton([this](){
                settings->writeSettingsFile();
                quit = true;
            });

            pipe.push_back(e3);

            break;
        }
    }
}

void Gui::folderCallback(void* userData, const char* const* files, int filter) {
    // TODO: handle closing or not picking folder
    Gui* self = static_cast<Gui*>(userData);
    if (self->currentSetting = SettingsE::OUTPUTFOLDER) {
        std::stringstream ss;
        ss << files[0];
        ss << "\\";

        self->settings->outputFolder = ss.str();
        std::cout << self->settings->outputFolder << std::endl;
    }
}


void Gui::kill() {
    quit = true;
}