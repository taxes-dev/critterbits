#pragma once
#ifndef CBENTITY_HPP
#define CBENTITY_HPP

#include <SDL.h>

#include <cassert>
#include <memory>

#include "coord.hpp"

namespace Critterbits {

enum class EntityType { Undefined, Sprite, Tilemap, Viewport, GuiPanel };
enum class EntityState { New, Active, Inactive, Unloaded };
typedef unsigned long entity_id_t;

#define CB_ENTITY_ID_INVALID 0L
#define CB_ENTITY_ID_FIRST 1L

extern entity_id_t next_entity_id;

namespace Scripting {
class Script;
}

class Entity : public std::enable_shared_from_this<Entity> {
    friend class Engine;

  public:
    const entity_id_t entity_id{next_entity_id++};
    CB_Rect dim{0, 0, 0, 0};
    std::string tag;
    std::shared_ptr<Scripting::Script> script;
    float time_scale{1.0f};
    EntityState state{EntityState::New};

    virtual EntityType GetEntityType() const = 0;
    bool HasScript() { return this->script != nullptr; };
    bool IsActive() { return this->state == EntityState::Active && this->destroyed == false; };
    void MarkDestroy() { this->destroyed = true; };
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);
    virtual void SetPosition(int x, int y) {
      if (this->IsActive()) {
        this->dim.x = x;
        this->dim.y = y;
      }
    };
    void Start();
    void Update(float);
    virtual ~Entity(){};

  protected:
    bool debug{false};

    Entity(){};
    virtual bool OnStart() { return true; };
    virtual void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &) {};
    virtual void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &) {};
    virtual void OnUpdate(float){};

  private:
    bool started{false};
    bool destroyed{false};
};
}
#endif