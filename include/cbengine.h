#pragma once
#ifndef CBENGINE_H
#define CBENGINE_H

#include <SDL.h>
#include <string>

#include "cbscene.h"
#include "cbviewport.h"

#define CB_DEFAULT_WINDOW_W 1024
#define CB_DEFAULT_WINDOW_H 768
#define CB_CONFIG_YAML "cbconfig.yml"
#define CB_DEFAULT_ASSET_PATH "./assets"

namespace Critterbits {

// messy, but some things need direct access to the renderer
extern SDL_Renderer * cb_main_renderer;

class EngineConfiguration {
  public:
    std::string asset_path;
    bool draw_debug_pane = false;
    bool draw_debug_sprite_rects = false;
    bool draw_map_regions = false;
    int window_width = CB_DEFAULT_WINDOW_W;
    int window_height = CB_DEFAULT_WINDOW_H;
    std::string window_title;

    EngineConfiguration(){};
    EngineConfiguration(const std::string &);
    inline bool IsValid() const { return this->valid; };
    bool ReloadConfiguration();

  private:
    bool valid = false;
};

class Engine {
  public:
    EngineConfiguration config;
    SDL_Rect display_bounds;
    SceneManager scenes;
    Viewport viewport;
    float fps;

    Engine(EngineConfiguration & engine_config) : config(engine_config){};
    ~Engine();
    int Run();

  private:
    SDL_Window * window = nullptr;

    Engine(const Engine &) = delete;
    Engine(Engine &&) = delete;
    void RenderDebugPane(SDL_Renderer * renderer, int, int);
};
}
#endif