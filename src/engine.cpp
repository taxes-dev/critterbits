#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {
Engine::~Engine() {
    SDLx::SDL_CleanUp(this->renderer, this->window);
    Tilemap::Tilemap_Quit();
    IMG_Quit();
    SDL_Quit();
}

Engine & Engine::GetInstance() {
    static Engine instance;
    return instance;
}

int Engine::Run() {
    LOG_INFO("Entering Engine::Run()");

    // check that we have valid configuration
    if (!this->config->IsValid()) {
        LOG_ERR("Engine::Run engine configuration is not valid");
        return 1;
    }

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO /*| SDL_INIT_TIMER*/) != 0) {
        LOG_SDL_ERR("Engine::Run SDL_Init");
        return 1;
    }
    if (!TestBitMask<int>(IMG_Init(IMG_INIT_PNG), IMG_INIT_PNG)) {
        LOG_SDL_ERR("Engine::Run IMG_Init");
        return 1;
    }

    // initialize TMX library
    Tilemap::Tilemap_Init();

    // discover display bounds
    SDL_GetDisplayBounds(0, &this->display_bounds);
    LOG_INFO("Engine::Run current display size " + std::to_string(this->display_bounds.w) + "x" +
             std::to_string(this->display_bounds.h));
    if (this->config->window.full_screen) {
        // create full screen window
        this->window = SDL_CreateWindow(this->config->window.title.c_str(), 0, 0, this->display_bounds.w,
                                        this->display_bounds.h, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
        this->config->window.width = this->display_bounds.w;
        this->config->window.height = this->display_bounds.h;
    } else {
        // create window of desired size in center of screen
        LOG_INFO("Engine::Run desired window size " + std::to_string(this->config->window.width) + "x" +
                 std::to_string(this->config->window.height));
        CB_Point window_origin = CB_Point::CenterInside(this->display_bounds.w, this->display_bounds.h,
                                                        this->config->window.width, this->config->window.height);

        this->window = SDL_CreateWindow(this->config->window.title.c_str(), window_origin.x, window_origin.y,
                                        this->config->window.width, this->config->window.height, SDL_WINDOW_SHOWN);
    }
    if (this->window == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateWindow");
        return 1;
    }

    // configure viewport
    this->viewport.dim = {0, 0, this->config->window.width, this->config->window.height};

    // create renderer
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderer == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateRenderer");
        return 1;
    }

    // get some information on the renderer we just created
    SDL_RendererInfo r_info;
    if (SDL_GetRendererInfo(this->renderer, &r_info) != 0) {
        LOG_SDL_ERR("Engine::Run SDL_GetRendererInfo");
        return 1;
    }

    LOG_INFO("Engine::Run renderer name " + std::string(r_info.name));
    if (TestBitMask(r_info.flags, (unsigned int)SDL_RENDERER_SOFTWARE)) {
        LOG_ERR("Engine::Run no hardware renderer available");
        return 1;
    }

    this->max_texture_height = r_info.max_texture_height;
    this->max_texture_width = r_info.max_texture_width;
    LOG_INFO("Engine::Run renderer max_texture_width " + std::to_string(r_info.max_texture_width));
    LOG_INFO("Engine::Run renderer max_texture_height " + std::to_string(r_info.max_texture_height));

    // load first scene
    if (!this->scenes.LoadScene(CB_FIRST_SCENE)) {
        LOG_ERR("Engine::Run cannot load startup scene");
        return 1;
    }

    // entities to iterate
    std::list<Entity *> entities;

    // timer
    unsigned int ticks;
    unsigned int frame_time;
    unsigned int frame_count = 0;
    float delta_time = 0.;
    int render_count = 0;

    // HACK
    std::shared_ptr<Sprite> player_sprite;
    CB_Point center_view{0, 0};

    // start main loop
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        ticks = SDL_GetTicks();
        frame_count++;

        while (SDL_PollEvent(&e)) {
            // If user closes the window
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN && e.key.state == SDL_PRESSED && player_sprite != nullptr) {
                const int player_velocity = 400;
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        player_sprite->dim.x -= delta_time * player_velocity;
                        player_sprite->SetFrame(0);
                        player_sprite->flip_x = false;
                        break;
                    case SDLK_RIGHT:
                        player_sprite->dim.x += delta_time * player_velocity;
                        player_sprite->SetFrame(0);
                        player_sprite->flip_x = true;
                        break;
                    case SDLK_UP:
                        player_sprite->dim.y -= delta_time * player_velocity;
                        player_sprite->SetFrame(6);
                        break;
                    case SDLK_DOWN:
                        player_sprite->dim.y += delta_time * player_velocity;
                        player_sprite->SetFrame(3);
                        break;
                }
            }
        }

        // Execute pre-update events
        EngineEventQueue::GetInstance().ExecutePreUpdate();

        // HACK
        if (player_sprite == nullptr) {
            player_sprite = *this->scenes.current_scene->sprites.sprites.begin();
            this->viewport.SetEntityToFollow(std::dynamic_pointer_cast<Entity>(player_sprite));
        }

        // Set list of entities to iterate
        entities.clear();
        if (this->scenes.current_scene != nullptr && this->scenes.current_scene->state == CBE_SCENE_ACTIVE) {
            Entity * tilemap = this->scenes.current_scene->GetTilemap();
            if (tilemap != nullptr) {
                entities.push_back(tilemap);
            }
            for (auto & sprite : this->scenes.current_scene->sprites.sprites) {
                entities.push_back(sprite.get());
            }
        }
        entities.push_back(&this->viewport);

        // Update cycle
        for (auto & entity : entities) {
            entity->Update(delta_time);
        }

        // Render pass
        SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
        SDL_RenderClear(this->renderer);
        render_count = 0;
        for (auto & entity : entities) {
            if (entity->dim.intersects(this->viewport.dim)) {
                entity->Render(this->renderer, this->viewport.GetViewableRect(entity->dim));
                render_count++;
            }
        }

        if (this->config->debug.draw_info_pane) {
            this->RenderDebugPane(entities.size(), render_count);
        }
        SDL_RenderPresent(this->renderer);

        frame_time = SDL_GetTicks() - ticks;
        delta_time = (float)frame_time / 1000.;
        if (frame_count % 10 == 0) {
            this->fps = 1000. / (float)frame_time;
        }
    }

    LOG_INFO("Exiting Engine::Run()");
    return 0;
}

void Engine::RenderDebugPane(int entity_count, int entities_in_view) {
    std::stringbuf info;
    std::ostream os(&info);

    os << "view" << this->viewport.dim.xy().to_string();
    os << " ent(" << entities_in_view << "/" << entity_count << ")";
    os << " fps " << std::fixed << std::setprecision(1) << this->fps;

    roundedBoxRGBA(this->renderer, -6, this->viewport.dim.h - 12, info.str().length() * 8 + 10,
                   this->viewport.dim.h + 6, 6, 0, 0, 0, 127);
    stringRGBA(this->renderer, 2, this->viewport.dim.h - 10, info.str().c_str(), 255, 255, 255, 255);
}

void Engine::SetConfiguration(std::shared_ptr<EngineConfiguration> config) { this->config = std::move(config); }
}