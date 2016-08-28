#include <SDL.h>
#include <iostream>

#include "cbengine.h"
#include "cbsdl.h"

namespace Critterbits {

Engine::~Engine() {
    SDL::SDL_CleanUp(this->window);
    SDL_Quit();
}

int Engine::Run() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    this->window = SDL_CreateWindow("Critterbits", 100, 100, this->window_width, this->window_height, SDL_WINDOW_SHOWN);
    if (this->window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            // If user closes the window
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    return 0;
}
}