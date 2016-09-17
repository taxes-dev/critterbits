#pragma once
#ifndef CBENTITY_HPP
#define CBENTITY_HPP

#include <SDL.h>

#include <cassert>
#include <memory>

#include "cbcoord.hpp"

namespace Critterbits {

enum class EntityType { Undefined, Sprite, Tilemap, Viewport };
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

    /* derived classes MUST implement GetEntityType() - not pure virtual due to dynamic casting constraints */
    virtual EntityType GetEntityType() {
        assert(false);
        return EntityType::Undefined;
    };

    bool HasScript() { return this->script != nullptr; };
    void MarkDestroy() { this->destroyed = true; };
    virtual void Render(SDL_Renderer *, const CB_ViewClippingInfo &){};
    virtual void SetPosition(int x, int y) {
        this->dim.x = x;
        this->dim.y = y;
    };
    void Start();
    void Update(float);
    virtual ~Entity(){};

  protected:
    Entity(){};
    virtual bool OnStart() { return true; };
    virtual void OnUpdate(float){};

  private:
    bool started{false};
    bool destroyed{false};
};
}
#endif