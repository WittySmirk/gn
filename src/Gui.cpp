#include "Gui.h"

Gui::Gui() {}

void Gui::spawn(Settings* _settings, bool _setup) {
    settings = _settings;
    // TODO: better state management
    if (_setup) {
        state = State::SETUPSTAGE1;
    } else {
        state = State::EDITINGSTAGE1;
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
        case State::EDITINGSTAGE1:
            SDL_ShowOpenFileDialog(fileCallback, this, window, nullptr, 0, settings->outputFolder.c_str(), false);
        break;
        case State::EDITINGSTAGE2:
            editor = new Editor(renderer, font);
            pipe.push_back(editor);
            editor->init(editFile);
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
            if(event.type == SDL_EVENT_TEXT_INPUT) {
                for(Element* e : pipe) {
                    e->collectText(event.text.text);
                }
            }
            if (state == State::EDITINGSTAGE2) {
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    switch(event.key.key) {
                        case SDLK_E:
                            editor->exportClip();
                            break;
                        case SDLK_M:
                            editor->createMarker();
                            break;
                        case SDLK_ESCAPE:
                            if(editor->getFocused()){
                                SDL_StopTextInput(window);
                            }
                            editor->clearMarkers();
                            break;

                        case SDLK_BACKSPACE:
                            for(Element* e : pipe) {
                                if(e->getFocused()) {
                                    e->deleteText();
                                }
                            }
                            break;

                        case SDLK_RETURN:
                            editor->completeExport();
                            break;
                    }
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

        if(state == State::EDITINGSTAGE2) {
            editor->read();
            editor->renderVideo();
        }

        SDL_SetRenderDrawColor(renderer, BACKGROUND.r, BACKGROUND.g, BACKGROUND.b, BACKGROUND.a);
        SDL_RenderClear(renderer);

        for (Element* e : pipe) {
            e->draw();
        }

        for(Element* e : pipe) {
            if(e->getFocused()) 
                SDL_StartTextInput(window);
            if(e->hasOverlay())
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
            Text* t1 = new Text(renderer, font, "First Time?", 1.0f, 48, FOREGROUND);
            pipe.push_back(t1);

            Text* t2 = new Text(renderer, font, "Setup", 1.2f, 34, FOREGROUND);
            t2->setBorderColor(HIGHLIGHT);
            t2->setButton([this](){
                state = State::SETUPSTAGE2;
            });
            pipe.push_back(t2);

            break;
        }
        case State::SETUPSTAGE2:{
            settings->detectGPU();
            const int padding = 25;

            Text* t1 = new Text(renderer, font, "Setup", padding, padding, 48, FOREGROUND);
            pipe.push_back(t1);

            Text* t2 = new Text(renderer, font, "Pick Capture Location", padding, t1->getRect()->y + t1->getRect()->h + padding, 34, FOREGROUND);
            t2->setButton([this, t2](){
                currentSetting = SettingsE::OUTPUTFOLDER;
                SDL_ShowOpenFolderDialog(folderCallback, this, window, nullptr, false);
            });
            t2->setBorderColor(HIGHLIGHT);
            pipe.push_back(t2);

            MultiSelect* m = new MultiSelect(renderer, font, "Select fps", std::vector<MultiItem>{
                MultiItem{"30 fps", [this](){settings->fps = 30;}},
                MultiItem{"60 fps", [this](){settings->fps = 60;}},
                MultiItem{"120 fps", [this](){settings->fps = 120;}}
            }, 
                padding, t2->getRect()->y + t2->getRect()->h + padding);
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

            MultiSelect* m2 = new MultiSelect(renderer, font, "Select Stereo Device", audioItems, padding, m->getRect()->y + m->getRect()->h + padding);
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

            MultiSelect* m3 = new MultiSelect(renderer, font, "Select Mic", audioItems, padding, m2->getRect()->y + m->getRect()->h + padding);
            pipe.push_back(m3);

            std::string gpu;
            if (settings->amd) {
                gpu = "Detected AMD gpu";
            } else if (settings->nvidia) {
                gpu = "Detected NVIDIA gpu";
            } else {
                gpu = "No gpu detected";
            }

            Text* t3 = new Text(renderer, font, gpu, padding, SCREEN_H - padding * 2, 34, HIGHLIGHT);
            pipe.push_back(t3);


            Text* error = new Text(renderer, font, "Please set all before finishing setup", padding, t3->getRect()->y - t3->getRect()->h - padding, 34, FOREGROUND);
            error->setRendered(false);
            pipe.push_back(error);

            Text* t4 = new Text(renderer, font, "Finished", padding, m3->getRect()->y + m3->getRect()->h + padding, 34, FOREGROUND);
            t4->setBorderColor(HIGHLIGHT);
            t4->setButton([this, error](){
                if(!settings->writeSettingsFile()) {
                    error->setRendered(true);
                    return;
                }
                quit = true;
            });
            pipe.push_back(t4);


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

void Gui::fileCallback(void* userData, const char* const* files, int filter) {
    // TODO: handle closing or not picking file
    Gui* self = static_cast<Gui*>(userData);
    self->editFile = files[0];
    self->state = State::EDITINGSTAGE2;
}

void Gui::kill() {
    quit = true;
}