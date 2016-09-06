#pragma once
#ifndef CBENGINE_H
#define CBENGINE_H

#include <SDL.h>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "cbinput.h"
#include "cbscene.h"
#include "cbscripting.h"
#include "cbviewport.h"

#define CB_DEFAULT_WINDOW_W 1024
#define CB_DEFAULT_WINDOW_H 768
#define CB_CONFIG_YAML "cbconfig.yml"
#define CB_DEFAULT_ASSET_PATH "./assets"
#define CB_DESIRED_UPS 60.0f

namespace Critterbits {

class EngineConfiguration {
  public:
    std::string asset_path;
    struct {
        bool draw_info_pane = false;
        bool draw_map_regions = false;
        bool draw_sprite_rects = false;
    } debug;
    struct {
        bool full_screen = false;
        int width = CB_DEFAULT_WINDOW_W;
        int height = CB_DEFAULT_WINDOW_H;
        std::string title;
    } window;

    EngineConfiguration(){};
    EngineConfiguration(const std::string &);
    inline bool IsValid() const { return this->valid; };
    bool ReloadConfiguration();
    bool Validate();

  private:
    bool valid = false;
};

class EngineCounters {
  public:
    EngineCounters() {};

    float GetAverageFps() { return this->fps; };
    float GetDeltaFromRemainingFrameTime();
    float GetDeltaTime() { return this->delta_time; };
    unsigned int GetRemainingFrameTime() { return this->frame_time; };
    unsigned int GetRenderedEntitiesCount() { return this->render_count; };
    void NewFrame();
    void RenderedEntity();
    void Reset();
    void Updated();

  private:
    const float delta_time = 1.0f / CB_DESIRED_UPS;

    float fps = 0.;
    unsigned int ticks = 0;
    unsigned int last_ticks = 0;
    unsigned int frame_time = 0;
    unsigned int frame_count = 0;
    unsigned int render_count = 0;
    unsigned int update_count = 0;
};

class Engine {
  public:
    std::shared_ptr<EngineConfiguration> config;
    SDL_Rect display_bounds;
    EngineCounters counters;
    InputManager input;
    ScriptEngine scripts;
    SceneManager scenes;
    Viewport viewport;

    ~Engine();
    static Engine & GetInstance();
    int GetMaxTextureHeight() const { return this->max_texture_height; };
    int GetMaxTextureWidth() const { return this->max_texture_width; };
    SDL_Renderer * GetRenderer() const { return this->renderer; };
    int Run();
    void SetConfiguration(std::shared_ptr<EngineConfiguration>);

  private:
    SDL_Window * window = nullptr;
    SDL_Renderer * renderer = nullptr;
    int max_texture_height = 0;
    int max_texture_width = 0;
    bool initialized = false;

    Engine();
    Engine(const Engine &) = delete;
    Engine(Engine &&) = delete;
    void operator=(Engine const &) = delete;
    void RenderDebugPane(int);
};

typedef std::function<void()> PreUpdateEvent;

class EngineEventQueue {
    friend Engine;

  public:
    ~EngineEventQueue(){};
    static EngineEventQueue & GetInstance();
    void QueuePreUpdate(const PreUpdateEvent & event);

  protected:
    void ExecutePreUpdate();

  private:
    // FIXME: I need to figure out why changing this to std::vector results in a random malloc segfault
    std::list<PreUpdateEvent> pre_update;

    EngineEventQueue(){};
    EngineEventQueue(const EngineEventQueue &) = delete;
    EngineEventQueue(EngineEventQueue &&) = delete;
    void operator=(EngineEventQueue const &) = delete;
};
}
#endif