#pragma once
#ifndef CBENGINE_HPP
#define CBENGINE_HPP

#include <SDL.h>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "resource.hpp"
#include "input.hpp"
#include "scene.hpp"
#include "viewport.hpp"
#include "gui.hpp"
#include "scripting/scripting.hpp"

#define CB_DEFAULT_WINDOW_W 1024
#define CB_DEFAULT_WINDOW_H 768
#define CB_DEFAULT_WINDOW_TITLE "Critterbits"
#define CB_CONFIG_FILE "cbconfig.toml"
#define CB_DEFAULT_ASSET_PATH "./assets"
#define CB_DESIRED_UPS 60.0f

namespace Critterbits {
template<class T>
using EntityIterateFunction = std::function<bool(std::shared_ptr<T>)>;

class EngineConfiguration {
  public:
    std::string asset_path;
    std::shared_ptr<ResourceLoader> loader;
    struct {
        bool draw_info_pane{false};
        bool draw_map_regions{false};
        bool draw_sprite_rects{false};
    } debug;
    struct {
        bool controller{false};
        bool keyboard{true};
        bool mouse{false};
    } input;
    struct {
        bool full_screen{false};
        int width{CB_DEFAULT_WINDOW_W};
        int height{CB_DEFAULT_WINDOW_H};
        std::string title{CB_DEFAULT_WINDOW_TITLE};
    } window;

    EngineConfiguration(){};
    EngineConfiguration(const std::string &);
    inline bool IsValid() const { return this->valid; };
    bool ReloadConfiguration();
    bool Validate();

  private:
    bool valid{false};

    std::string GetExpandedPath(const std::string &);
};

class EngineCounters {
  public:
    EngineCounters() { this->Reset(); };

    void CountedEntity();
    float GetAverageFps() { return this->fps; };
    float GetDeltaFromRemainingFrameTime();
    float GetDeltaTime() { return this->delta_time; };
    unsigned int GetRemainingFrameTime() { return this->frame_time; };
    unsigned int GetRenderedEntitiesCount() { return this->render_count; };
    unsigned int GetTotalEntitiesCount() { return this->entity_count; };
    void NewFrame();
    void RenderedEntity();
    void Reset();
    void Updated();

  private:
    const float delta_time{1.0f / CB_DESIRED_UPS};

    float fps;
    unsigned int ticks;
    unsigned int last_ticks;
    unsigned int frame_time;
    unsigned int frame_count;
    unsigned int entity_count;
    unsigned int render_count;
    unsigned int update_count;
};

class Engine {
  public:
    std::shared_ptr<EngineConfiguration> config;
    CB_Rect display_bounds;
    EngineCounters counters;
    InputManager input;
    Scripting::ScriptEngine scripts;
    SceneManager scenes;
    Gui::GuiManager gui;
    std::shared_ptr<Viewport> viewport{std::make_shared<Viewport>()};

    ~Engine();
    std::shared_ptr<Entity> FindEntityById(entity_id_t);
    std::vector<std::shared_ptr<Entity>> FindEntitiesByTag(const std::string &);
    static Engine & GetInstance();
    int GetMaxTextureHeight() const { return this->max_texture_height; };
    int GetMaxTextureWidth() const { return this->max_texture_width; };
    std::shared_ptr<ResourceLoader> GetResourceLoader() const;
    SDL_Renderer * GetRenderer() const { return this->renderer; };
    void IterateEntities(EntityIterateFunction<Entity>);
    void IterateActiveEntities(EntityIterateFunction<Entity>);
    void IterateActiveGuiPanels(EntityIterateFunction<Gui::GuiPanel>, bool = false);
    void IterateActiveSprites(EntityIterateFunction<Sprite>);
    int Run();
    void SetConfiguration(std::shared_ptr<EngineConfiguration>);

  private:
    SDL_Window * window{nullptr};
    SDL_Renderer * renderer{nullptr};
    int max_texture_height{0};
    int max_texture_width{0};
    bool initialized{false};

    Engine();
    Engine(const Engine &) = delete;
    Engine(Engine &&) = delete;
    void operator=(Engine const &) = delete;
    void DestroyMarkedEntities();
    void RenderDebugPane(int);
};

class Sprite;

typedef std::function<void()> PreUpdateEvent;
typedef std::function<void()> CollisionEvent;

class EngineEventQueue {
    friend Engine;

  public:
    ~EngineEventQueue(){};
    static EngineEventQueue & GetInstance();
    void QueueCollision(const CollisionEvent);
    void QueuePreUpdate(const PreUpdateEvent);

  protected:
    void ExecuteCollision();
    void ExecutePreUpdate();

  private:
    std::vector<CollisionEvent> collision;
    std::vector<PreUpdateEvent> pre_update;

    EngineEventQueue(){};
    EngineEventQueue(const EngineEventQueue &) = delete;
    EngineEventQueue(EngineEventQueue &&) = delete;
    void operator=(EngineEventQueue const &) = delete;
};
}
#endif