#pragma once
#ifndef CBENGINE_H
#define CBENGINE_H

#include <SDL.h>
#include <string>

#define CB_DEFAULT_WINDOW_W 1024
#define CB_DEFAULT_WINDOW_H 768
#define CB_CONFIG_YAML "cbconfig.yml"
#define CB_DEFAULT_ASSET_PATH "./assets"

namespace Critterbits {
class EngineConfiguration {
  public:
    std::string asset_path;
    int window_width = CB_DEFAULT_WINDOW_W;
    int window_height = CB_DEFAULT_WINDOW_H;
    std::string window_title;

    EngineConfiguration(){};
    EngineConfiguration(const std::string &);
    inline bool is_valid() const { return this->valid; };
    bool ReloadConfiguration();

  private:
    bool valid = false;
};

class Engine {
  public:
    EngineConfiguration config;
    SDL_Rect display_bounds;

    Engine(EngineConfiguration & engine_config) : config(engine_config){};
    ~Engine();
    int Run();

  private:
    SDL_Window * window = nullptr;
    SDL_Renderer * renderer = nullptr;

    Engine(const Engine &) = delete;
    Engine(Engine &&) = delete;
};
}
#endif