#include <SDL.h>
#include <iostream>

#include "cbcoord.h"
#include "cbengine.h"
#include "cblogging.h"
#include "cbsdl.h"

namespace Critterbits {

Engine::~Engine() {
    SDL::SDL_CleanUp(renderer, window);
    SDL_Quit();
}

int Engine::Run() {
    LOG_INFO("Entering Engine::Run()");

    // check that we have valid configuration
    if (!this->config.is_valid()) {
        LOG_ERR("Engine configuration is not valid");
        return 1;
    }
    LOG_INFO("Final asset path: " + config.asset_path);
    LOG_INFO("Final window width: " + std::to_string(config.window_width));
    LOG_INFO("Final window height: " + std::to_string(config.window_height));

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_SDL_ERR("SDL_Init");
        return 1;
    }

    // discover center of screen for window display
    SDL_GetDisplayBounds(0, &this->display_bounds);
    CB_Point window_origin =
        center_inside(display_bounds.w, display_bounds.h, config.window_width, config.window_height);

    // create display window
    this->window = SDL_CreateWindow("Critterbits", window_origin.x, window_origin.y, config.window_width,
                                    config.window_height, SDL_WINDOW_SHOWN);
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
    }

    LOG_INFO("Exiting Engine::Run()");
    return 0;
}
}