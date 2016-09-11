#pragma once
#ifndef CBENTITY_H
#define CBENTITY_H

#include <SDL.h>

#include <cassert>
#include <memory>

#include "cbcoord.h"

namespace Critterbits {

typedef enum { CBE_UNDEFINED, CBE_SPRITE, CBE_TILEMAP, CBE_VIEWPORT } EntityType;
typedef unsigned long entity_id_t;

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
        return CBE_UNDEFINED;
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