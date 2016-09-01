#pragma once
#ifndef CBENGINE_H
#define CBENGINE_H

#include <SDL.h>
#include <memory>
#include <string>

#include "cbscene.h"
#include "cbviewport.h"

#define CB_DEFAULT_WINDOW_W 1024
#define CB_DEFAULT_WINDOW_H 768
#define CB_CONFIG_YAML "cbconfig.yml"
#define CB_DEFAULT_ASSET_PATH "./assets"

namespace Critterbits {

class EngineConfiguration {
  public:
    std::string asset_path;
    struct {
        bool draw_info_pane = false;
        bool draw_map_regions = false;
        bool draw_sprite_rects = false;
    } debug;
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
    std::shared_ptr<EngineConfiguration> config;
    SDL_Rect display_bounds;
    SceneManager scenes;
    Viewport viewport;
    float fps;

    ~Engine();
    static Engine & GetInstance();
    SDL_Renderer * GetRenderer() const { return this->renderer; }
    int Run();
    void SetConfiguration(std::shared_ptr<EngineConfiguration>);

  private:
    SDL_Window * window = nullptr;
    SDL_Renderer * renderer = nullptr;

    Engine(){};
    Engine(const Engine &) = delete;
    Engine(Engine &&) = delete;
    void operator=(Engine const &) = delete;
    void RenderDebugPane(int, int);
};
}
#endif