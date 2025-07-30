#include "Settings.h"

Settings::Settings() {}

bool Settings::readSettings() {
    std::ifstream in("Settings");
    if(!in) {
        return false;
    }
    std::string line;
    
    int category = -1;
    int type = 0;

    while(std::getline(in, line)) {
        if(line.empty())
            continue;
        if(line.at(0) == '[') {
            category++;
            type = 0;
            continue;
        }
        std::stringstream afterS;
        bool before = true;
        for (int i = 0; i < line.length(); i++) {
            if (line.at(i) == '=') {
                before = false;
                continue;
            }
            if(!before)
                afterS << line.at(i);
        }
        
        if(category == 0) {
            switch(type) {
                case 0: afterS >> nvidia; break;
                case 1: afterS >> amd; break;
            }
        } else {
            switch(type) {
                case 0: afterS >> fps; break;
                case 1: outputFolder = afterS.str(); break;
                case 2: clipsFolder = afterS.str(); break;
            }
        }

        type++;
    }
    in.close();
    return true;
}

bool Settings::writeSettingsFile() {
std::cout << "FPS: " << fps << "OUT FOLDER: " << outputFolder << "CLIPSFOLDER: " << clipsFolder;
    if(fps == NULL || outputFolder == "" || clipsFolder == "")
        return false;
    
    std::ofstream out("Settings");
    if(!out) {
        std::cerr << "output didn't work" << std::endl;
        return false;
    }

    out << "[HARDWARE]" << std::endl;
    out << "nvidia=" << nvidia << std::endl;
    out << "amd=" << amd << std::endl;
    //out << "mic=" << mic << std::endl;
    out << "\n";
    out << "[SETTINGS]" << std::endl;
    out << "fps=" << fps << std::endl;
    out << "outputFolder=" << outputFolder << std::endl;
    out << "clipsFolder=" << clipsFolder << std::endl;
    
    out.close();
    return true;
}

void Settings::detectGPU() {
    SDL_Window* window = SDL_CreateWindow("gpu", 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if(!window) {
        std::cerr << "SDL create window error" << std::endl;
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context) {
        std::cerr << "Could not create the OpenGL context" << std::endl;
        exit(1);
    }

    std::string gpu = (const char*) glGetString(GL_VENDOR);

    if(gpu.find("NVIDIA") != std::string::npos) {
        std::cout << "Nvidia gpu!" << std::endl;
        nvidia = true;
    } else if((gpu.find("ATI") != std::string::npos) || (gpu.find("Advanced") != std::string::npos)) {
        std::cout << "Amd gpu!" << std::endl;
        amd = true;
    }

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
}