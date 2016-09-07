#pragma once
#ifndef CBENTITY_H
#define CBENTITY_H

#include <cassert>

#include "cbcoord.h"
#include <SDL.h>

namespace Critterbits {

typedef enum { CBE_UNDEFINED, CBE_SPRITE, CBE_TILEMAP, CBE_VIEWPORT } EntityType;
typedef unsigned long entity_id_t;

extern entity_id_t next_entity_id;

class Script;

class Entity {
    friend class Engine;

  public:
    const entity_id_t entity_id{next_entity_id++};
    CB_Rect dim{0, 0, 0, 0};
    std::string tag;
    std::shared_ptr<Script> script;
    float time_scale{1.0f};

    /* derived classes MUST implement GetEntityType() - not pure virtual due to dynamic casting constraints */
    virtual EntityType GetEntityType() {
        assert(false);
        return CBE_UNDEFINED;
    };

    bool HasScript() { return this->script != nullptr; };
    void MarkDestroy() { this->destroyed = true; };
    virtual void Render(SDL_Renderer *, const CB_ViewClippingInfo &){};
    virtual void SetPosition(int x,int y) { this->dim.x = x; this->dim.y = y; };
    virtual void Start() { this->started = true; };
    virtual void Update(float){};
    virtual ~Entity(){};

  protected:
    Entity(){};

  private:
    bool started{false};
    bool destroyed{false};
};
}
#endif