#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include <cb/critterbits.hpp>
#include <cb/memory/nadeau.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {

Engine::Engine() {
    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_SDL_ERR("Engine::Engine SDL_Init");
    } else {
        this->initialized = true;
    }
}

Engine::~Engine() {
    SDLx::SDL_CleanUp(this->renderer, this->window);
    SDL_Quit();
}

bool Engine::ConfigureManagers() {
    // configure texture manager
    this->textures.SetResourceLoader(this->config->loader);

    // configure font manager
    this->fonts.SetResourceLoader(this->config->loader);
    for (auto & named_font : this->config->configured_fonts) {
        this->fonts.RegisterNamedFont(named_font);
    }

    // setup input methods
    this->input.SetControllerActive(this->config->input.controller);
    this->input.SetKeyboardActive(this->config->input.keyboard);
    this->input.SetMouseActive(this->config->input.mouse);

    // start scripting engine
    this->scripts.StartEngine();

    return false;
}

bool Engine::CreateWindowAndRenderer() {
    // discover display bounds
    SDL_Rect disp_bounds;
    SDL_GetDisplayBounds(0, &disp_bounds);
    this->display_bounds = {disp_bounds.x, disp_bounds.y, disp_bounds.w, disp_bounds.h};
    LOG_INFO("Engine::CreateWindowAndRenderer current display size " + std::to_string(this->display_bounds.w) + "x" +
             std::to_string(this->display_bounds.h));
    if (this->config->window.full_screen) {
        // create full screen window
        this->window = SDL_CreateWindow(this->config->window.title.c_str(), 0, 0, this->display_bounds.w,
                                        this->display_bounds.h, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
        this->config->window.width = this->display_bounds.w;
        this->config->window.height = this->display_bounds.h;
    } else {
        // create window of desired size in center of screen
        LOG_INFO("Engine::CreateWindowAndRenderer desired window size " + std::to_string(this->config->window.width) + "x" +
                 std::to_string(this->config->window.height));
        CB_Point window_origin = CB_Point::CenterInside(this->display_bounds.w, this->display_bounds.h,
                                                        this->config->window.width, this->config->window.height);

        this->window = SDL_CreateWindow(this->config->window.title.c_str(), window_origin.x, window_origin.y,
                                        this->config->window.width, this->config->window.height, SDL_WINDOW_SHOWN);
    }
    if (this->window == nullptr) {
        LOG_SDL_ERR("Engine::CreateWindowAndRenderer SDL_CreateWindow");
        return true;
    }

    // configure viewport
    int viewport_w = static_cast<int>(static_cast<float>(this->config->window.width) / this->config->rendering.scale);
    int viewport_h = static_cast<int>(static_cast<float>(this->config->window.height) / this->config->rendering.scale);
    this->viewport->dim = {0, 0, viewport_w, viewport_h};

    // create renderer
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderer == nullptr) {
        LOG_SDL_ERR("Engine::CreateWindowAndRenderer SDL_CreateRenderer");
        return true;
    }

    // get some information on the renderer we just created
    SDL_RendererInfo r_info;
    if (SDL_GetRendererInfo(this->renderer, &r_info) != 0) {
        LOG_SDL_ERR("Engine::CreateWindowAndRenderer SDL_GetRendererInfo");
        return true;
    }

    LOG_INFO("Engine::CreateWindowAndRenderer renderer name " + std::string(r_info.name));
    if (TestBitMask<unsigned int>(r_info.flags, SDL_RENDERER_SOFTWARE)) {
        LOG_ERR("Engine::CreateWindowAndRenderer no hardware renderer available");
        return true;
    }

    this->max_texture_height = r_info.max_texture_height;
    this->max_texture_width = r_info.max_texture_width;
    LOG_INFO("Engine::CreateWindowAndRenderer renderer max_texture_width " + std::to_string(r_info.max_texture_width));
    LOG_INFO("Engine::CreateWindowAndRenderer renderer max_texture_height " + std::to_string(r_info.max_texture_height));

    return false;
}

void Engine::DestroyMarkedEntities() {
    // copy all the entities to a vector first, as destruction will alter underlying collections
    std::vector<std::shared_ptr<Entity>> destroyed_entities;
    this->IterateEntities([&destroyed_entities](std::shared_ptr<Entity> entity) {
        if (entity->destroyed) {
            destroyed_entities.push_back(entity);
        }
        return false;
    });

    for (auto & entity : destroyed_entities) {
        switch (entity->GetEntityType()) {
            case EntityType::Sprite:
                if (this->scenes.IsCurrentSceneActive()) {
                    this->scenes.current_scene->sprites.UnloadSprite(std::dynamic_pointer_cast<Sprite>(entity));
                }
                break;
            case EntityType::GuiPanel:
                this->gui.UnloadPanel(std::dynamic_pointer_cast<Gui::GuiPanel>(entity));
                break;
            case EntityType::Tilemap:
            case EntityType::Viewport:
                // tilemaps and viewports should not be destroyed like this
                LOG_ERR("Engine::DestroyMarkedEntities something marked a tilemap or viewport for destruction");
                entity->destroyed = false;
                break;
            default:
                LOG_ERR("Engine::DestroyMarkedEntities unsupported entity type " +
                        std::to_string(static_cast<int>(entity->GetEntityType())));
                entity->destroyed = false;
                break;
        }
    }
}

std::shared_ptr<Entity> Engine::FindEntityById(entity_id_t entity_id) {
    std::shared_ptr<Entity> ret{nullptr};
    this->IterateActiveEntities([&](std::shared_ptr<Entity> entity) {
        if (entity->entity_id == entity_id) {
            ret = entity;
            return true;
        }
        return false;
    });
    return ret;
}

std::vector<std::shared_ptr<Entity>> Engine::FindEntitiesByTag(const std::string & tag) {
    std::vector<std::shared_ptr<Entity>> entities;
    if (!tag.empty()) {
        this->IterateActiveEntities([&](std::shared_ptr<Entity> entity) {
            if (entity->tag == tag) {
                entities.push_back(entity);
            }
            return false;
        });
    }
    return entities;
}

Engine & Engine::GetInstance() {
    static Engine instance;
    return instance;
}

std::shared_ptr<ResourceLoader> Engine::GetResourceLoader() const {
    if (this->config == nullptr || this->config->loader == nullptr) {
        LOG_ERR("Engine::GetResourceLoader called before resource loader is ready (programming error?)");
        return nullptr;
    }
    return this->config->loader;
}

void Engine::IterateEntities(EntityIterateFunction<Entity> func) {
    if (this->scenes.IsCurrentSceneActive()) {
        if (this->scenes.current_scene->HasTilemap()) {
            if (func(this->scenes.current_scene->GetTilemap())) {
                return;
            }
            for (auto & region : this->scenes.current_scene->GetTilemap()->regions) {
                if (func(region)) {
                    return;
                }
            }
        }
        for (auto & sprite : this->scenes.current_scene->sprites.sprites) {
            if (func(sprite)) {
                return;
            }
        }
    }
    for (auto & panel : this->gui.panels) {
        if (func(panel)) {
            return;
        }
        for (auto & control : panel->children) {
            if (func(control)) {
                return;
            }
        }
    }
    if (func(this->viewport)) {
        return;
    }
}

void Engine::IterateActiveColliders(EntityIterateFunction<BoxCollider> func) {
    EntityIterateFunction<BoxCollider> wrapper = [&func](std::shared_ptr<BoxCollider> collider) {
        if (collider->IsActive()) {
            return func(collider);
        }
        return false;
    };

    if (this->scenes.IsCurrentSceneActive()) {
        if (this->scenes.current_scene->HasTilemap()) {
            for (auto & region : this->scenes.current_scene->GetTilemap()->regions) {
                if (wrapper(region)) {
                    return;
                }
            }
        }
        for (auto & sprite : this->scenes.current_scene->sprites.sprites) {
            if (wrapper(sprite)) {
                return;
            }
        }
    }
}

void Engine::IterateActiveEntities(EntityIterateFunction<Entity> func) {
    EntityIterateFunction<Entity> wrapper = [&func](std::shared_ptr<Entity> entity) {
        if (entity->IsActive()) {
            return func(entity);
        }
        return false;
    };
    this->IterateEntities(wrapper);
}

void Engine::IterateActiveGuiPanels(EntityIterateFunction<Gui::GuiPanel> func) {
    EntityIterateFunction<Gui::GuiPanel> wrapper = [&func](std::shared_ptr<Gui::GuiPanel> panel) {
        if (panel->IsActive()) {
            return func(panel);
        }
        return false;
    };
    for (auto & panel : this->gui.panels) {
        if (func(panel)) {
            return;
        }
    }
}

void Engine::IterateActiveSprites(EntityIterateFunction<Sprite> func) {
    EntityIterateFunction<Sprite> wrapper = [&func](std::shared_ptr<Sprite> sprite) {
        if (sprite->IsActive()) {
            return func(sprite);
        }
        return false;
    };

    if (this->scenes.IsCurrentSceneActive()) {
        for (auto & sprite : this->scenes.current_scene->sprites.sprites) {
            if (wrapper(sprite)) {
                return;
            }
        }
    }
}

int Engine::Run() {
    LOG_INFO("Entering Engine::Run()");

    if (!this->initialized || !this->fonts.IsInitialized() || !this->textures.IsInitialized()) {
        LOG_ERR("Engine::Run engine initialization was unsuccessful");
        return 1;
    }

    // check that we have valid configuration
    if (!this->config->IsValid()) {
        LOG_ERR("Engine::Run engine configuration is not valid");
        return 1;
    }

    // create SDL window and renderer
    if (this->CreateWindowAndRenderer()) {
        LOG_ERR("Engine::Run unable to create SDL window/renderer");
        return 1;
    }

    // configure various manager classes
    if (this->ConfigureManagers()) {
        LOG_ERR("Engine::Run unable to configure managers");
        return 1;
    }

    // set icon on SDL window
    this->SetWindowIcon();

    // load first scene
    if (!this->scenes.LoadScene(CB_FIRST_SCENE)) {
        LOG_ERR("Engine::Run cannot load startup scene");
        return 1;
    }

    // start main loop
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        // timing update
        this->counters.NewFrame();

        // check for waiting SDL events
        while (SDL_PollEvent(&e)) {
            // If user closes the window
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            // InputManager will process the event if it's input-related
            this->input.AddSdlEvent(e);
        }

        // check input state (non-event)
        this->input.CheckInputs();

        // begin simulation loop
        while (this->counters.GetRemainingFrameTime() > 0) {
            float dt = this->counters.GetDeltaFromRemainingFrameTime();

            // Execute pre-update events
            EngineEventQueue::GetInstance().ExecutePreUpdate();

            // Update cycle
            this->IterateEntities([dt](std::shared_ptr<Entity> entity) {
                // start entity if it hasn't already
                entity->Start();

                // call frame update methods
                entity->Update(dt);

                return false;
            });

            // resolve collision events
            EngineEventQueue::GetInstance().ExecuteCollision();

            // timing update
            this->counters.Updated();
        }

        // Render pass
        if (this->scenes.IsCurrentSceneActive() && this->scenes.current_scene->HasTilemap()) {
            SDL_Color bg_color = this->scenes.current_scene->GetTilemap()->bg_color;
            SDL_SetRenderDrawColor(this->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        } else {
            SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
        }
        SDL_RenderClear(this->renderer);
        SDL_RenderSetScale(this->renderer, this->config->rendering.scale, this->config->rendering.scale);

        // render all active entities
        for (auto z_index : {ZIndex::Background, ZIndex::Midground, ZIndex::Foreground}) {
            this->IterateActiveEntities([this, z_index](std::shared_ptr<Entity> entity) {
                if (entity->dim.intersects(this->viewport->dim)) {
                    CB_ViewClippingInfo clip = this->viewport->GetViewableRect(entity->dim, z_index);
                    entity->Render(this->renderer, clip);
                    if (z_index == ZIndex::Background) {
                        // guard is to make sure we only count each entity rendered once per frame
                        this->counters.RenderedEntity();
                    }
                }
                if (z_index == ZIndex::Background) {
                    this->counters.CountedEntity();
                }
                return false;
            });
        }

        // finally render GUI on top of everything else
        CB_Rect gui_view{0, 0, this->viewport->dim.w, this->viewport->dim.h};
        this->IterateActiveGuiPanels([this, &gui_view](std::shared_ptr<Gui::GuiPanel> panel) {
            if (panel->dim.intersects(gui_view)) {
                CB_ViewClippingInfo clip = this->viewport->GetStaticViewableRect(panel->dim, ZIndex::Gui);
                panel->Render(this->renderer, clip);
            }
            return false;
        });

        if (this->config->debug.draw_info_pane) {
            SDL_RenderSetScale(this->renderer, 1.0f, 1.0f);
            this->RenderDebugPane();
        }
        SDL_RenderPresent(this->renderer);

        // Clean up entities that were marked for deletion
        this->DestroyMarkedEntities();
    }

    LOG_INFO("Exiting Engine::Run()");
    return 0;
}

void Engine::RenderDebugPane() {
    std::stringbuf info;
    std::ostream os(&info);

    float mem_mb_current = (float)NadeauSoftware::getCurrentRSS() / 1024.f / 1024.f;

    os << "view " << this->viewport->dim.xy().to_string();
    os << " | ent " << this->counters.GetRenderedEntitiesCount() << "/" << this->counters.GetTotalEntitiesCount();
    os << " | " << std::fixed << std::setprecision(1) << this->counters.GetAverageFps() << " fps";
    os << " | " << std::fixed << std::setprecision(2) << mem_mb_current << " MB";

    roundedBoxRGBA(this->renderer, -6, this->config->window.height - 12, info.str().length() * 8 + 10,
                   this->config->window.height + 6, 6, 0, 0, 0, 127);
    stringRGBA(this->renderer, 2, this->config->window.height - 10, info.str().c_str(), 255, 255, 255, 255);
}

void Engine::SetConfiguration(std::shared_ptr<EngineConfiguration> config) { this->config = std::move(config); }

void Engine::SetWindowIcon() {
    if (!this->config->window.icon_path.empty()) {
        std::shared_ptr<SDL_Surface> icon = this->config->loader->GetImageResourceAsSurface(this->config->window.icon_path);
        if (icon != nullptr) {
            SDL_SetWindowIcon(this->window, icon.get());
        } else {
            LOG_ERR("Engine::SetWindowIcon unable to get window icon");
        }
    }
}
}