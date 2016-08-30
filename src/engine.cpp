#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

#include "cbcoord.h"
#include "cbengine.h"
#include "cblogging.h"
#include "cbscene.h"
#include "cbsdl.h"
#include "cbtilemap.h"

namespace Critterbits {
// TODO: remove this
void renderTexture(SDL_Texture * tex, SDL_Renderer * ren, int x, int y) {
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    // Setup the destination rectangle to be at the position we want
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_RenderCopy(ren, tex, NULL, &dst);
}
SDL_Renderer * main_renderer = nullptr;

Engine::~Engine() {
    SDL::SDL_CleanUp(main_renderer, this->window);
    IMG_Quit();
    SDL_Quit();
}

int Engine::Run() {
    LOG_INFO("Entering Engine::Run()");

    // check that we have valid configuration
    if (!this->config.IsValid()) {
        LOG_ERR("Engine::Run engine configuration is not valid");
        return 1;
    }

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_SDL_ERR("Engine::Run SDL_Init");
        return 1;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        LOG_SDL_ERR("Engine::Run IMG_Init");
        return 1;
    }

    // initialize TMX library
    Tilemap::Tilemap_Init();

    // discover center of screen for window display
    SDL_GetDisplayBounds(0, &this->display_bounds);
    CB_Point window_origin = CB_Point::CenterInside(this->display_bounds.w, this->display_bounds.h,
                                                    this->config.window_width, this->config.window_height);

    // create display window
    this->window = SDL_CreateWindow(this->config.window_title.c_str(), window_origin.x, window_origin.y,
                                    this->config.window_width, this->config.window_height, SDL_WINDOW_SHOWN);
    if (this->window == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateWindow");
        return 1;
    }

    // create renderer
    main_renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (main_renderer == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateRenderer");
        return 1;
    }

    // configure scene manager and load first scene
    this->scenes.SetAssetPath(this->config.asset_path);
    if (!this->scenes.LoadScene(CB_FIRST_SCENE)) {
        LOG_ERR("Engine::Run cannot load startup scene");
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
        SDL_RenderClear(main_renderer);
        if (this->scenes.current_scene != nullptr) {
            SDL_Texture * tex = this->scenes.current_scene->GetMapTexture();
            if (tex != nullptr) {
                renderTexture(tex, main_renderer, 0, 0);
            }
        }

        SDL_RenderPresent(main_renderer);
    }

    LOG_INFO("Exiting Engine::Run()");
    return 0;
}
}