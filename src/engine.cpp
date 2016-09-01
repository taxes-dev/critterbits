#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {
SDL_Renderer * cb_main_renderer = nullptr;

Engine::~Engine() {
    SDLx::SDL_CleanUp(cb_main_renderer, this->window);
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
    if (!TestBitMask<int>(IMG_Init(IMG_INIT_PNG), IMG_INIT_PNG)) {
        LOG_SDL_ERR("Engine::Run IMG_Init");
        return 1;
    }

    // initialize TMX library
    Tilemap::Tilemap_Init(this->config.draw_map_regions);

    // set sprite debug
    cb_draw_debug_sprite_rects = this->config.draw_debug_sprite_rects;

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

    // configure viewport
    this->viewport.dim = {0, 0, this->config.window_width, this->config.window_height};

    // create renderer
    cb_main_renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (cb_main_renderer == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateRenderer");
        return 1;
    }

    // configure scene manager and load first scene
    this->scenes.SetAssetPath(this->config.asset_path);
    if (!this->scenes.LoadScene(CB_FIRST_SCENE)) {
        LOG_ERR("Engine::Run cannot load startup scene");
        return 1;
    }

    // entities to iterate
    std::list<Entity *> entities;

    // timer
    unsigned int ticks;
    unsigned int last_tick;
    unsigned int frame_count = 0;

    // HACK
    std::shared_ptr<Sprite> player_sprite = *this->scenes.current_scene->sprites.sprites.begin();
    CB_Point center_view =
        CB_Point::CenterInside(this->viewport.dim.w, this->viewport.dim.h, player_sprite->dim.w, player_sprite->dim.h);

    // start main loop
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        last_tick = ticks;
        frame_count++;

        while (SDL_PollEvent(&e)) {
            // If user closes the window
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN && e.key.state == SDL_PRESSED) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        player_sprite->dim.x -= 6;
                        player_sprite->SetFrame(0);
                        player_sprite->flip_x = false;
                        break;
                    case SDLK_RIGHT:
                        player_sprite->dim.x += 6;
                        player_sprite->SetFrame(0);
                        player_sprite->flip_x = true;
                        break;
                    case SDLK_UP:
                        player_sprite->dim.y -= 6;
                        player_sprite->SetFrame(6);
                        break;
                    case SDLK_DOWN:
                        player_sprite->dim.y += 6;
                        player_sprite->SetFrame(3);
                        break;
                }
            }
        }

        // Re-center viewport on player sprite
        this->viewport.dim.x = player_sprite->dim.x - center_view.x;
        this->viewport.dim.y = player_sprite->dim.y - center_view.y;

        // Cull entities not in viewport
        entities.clear();
        if (this->scenes.current_scene != nullptr && this->scenes.current_scene->state == CBE_ACTIVE) {
            Entity * tilemap = this->scenes.current_scene->GetTilemap();
            if (tilemap != nullptr && tilemap->dim.intersects(this->viewport.dim)) {
                entities.push_back(tilemap);
            }
        }
        for (auto & sprite : this->scenes.current_scene->sprites.sprites) {
            if (sprite->dim.intersects(this->viewport.dim)) {
                entities.push_back(sprite.get());
            }
        }

        // Render pass
        SDL_SetRenderDrawColor(cb_main_renderer, 0, 0, 0, 0);
        SDL_RenderClear(cb_main_renderer);
        for (auto & entity : entities) {
            entity->Render(cb_main_renderer, this->viewport.GetViewableRect(entity->dim));
        }

        if (this->config.draw_debug_pane) {
            this->RenderDebugPane(cb_main_renderer, entities.size(), entities.size());
        }

        SDL_RenderPresent(cb_main_renderer);

        ticks = SDL_GetTicks();
        if (frame_count % 10 == 0) {
            this->fps = 1000. / (float)(ticks - last_tick);
        }
    }

    LOG_INFO("Exiting Engine::Run()");
    return 0;
}

void Engine::RenderDebugPane(SDL_Renderer * renderer, int entity_count, int entities_in_view) {
    std::stringbuf info;
    std::ostream os(&info);

    os << "view" << this->viewport.dim.xy().to_string();
    os << " ent(" << entities_in_view << "/" << entity_count << ")";
    os << " fps " << std::fixed << std::setprecision(1) << this->fps;

    roundedBoxRGBA(renderer, -6, this->viewport.dim.h - 12, info.str().length() * 8 + 10, this->viewport.dim.h + 6, 6,
                   0, 0, 0, 127);
    stringRGBA(renderer, 2, this->viewport.dim.h - 10, info.str().c_str(), 255, 255, 255, 255);
}
}