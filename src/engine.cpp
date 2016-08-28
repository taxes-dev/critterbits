#include <SDL.h>
#include <iostream>

#include "cbengine.h"
#include "cblogging.h"
#include "cbsdl.h"

namespace Critterbits {

Engine::~Engine() {
    SDL::SDL_CleanUp(renderer, window);
    SDL_Quit();
}

int Engine::Run() {
    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_SDL_ERR("SDL_Init");
        return 1;
    }

    // discover center of screen for window display
    SDL_GetDisplayBounds(0, &this->display_bounds);

    int init_x = this->display_bounds.w / 2 - this->window_width / 2;
    int init_y = this->display_bounds.h / 2 - this->window_height / 2;

    // create display window
    this->window =
        SDL_CreateWindow("Critterbits", init_x, init_y, this->window_width, this->window_height, SDL_WINDOW_SHOWN);
    if (this->window == nullptr) {
        LOG_SDL_ERR("SDL_CreateWindow");
        return 1;
    }

    // create renderer
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderer == nullptr) {
        LOG_SDL_ERR("SDL_CreateRenderer");
        return 1;
    }

    // start main loop
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            // If user closes the window
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Render pass
        SDL_RenderClear(this->renderer);

        SDL_RenderPresent(this->renderer);
        SDL_Delay(1000);
    }

    return 0;
}
}