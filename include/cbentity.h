#pragma once
#ifndef CBENTITY_H
#define CBENTITY_H

#include <cassert>

#include <SDL.h>
#include "cbcoord.h"

namespace Critterbits {

typedef enum { CBE_SPRITE, CBE_TILEMAP, CBE_VIEWPORT } EntityType;

class Script;

class Entity {
  friend class Engine;

  public:
    CB_Rect dim = {0, 0, 0, 0};
    std::string tag;
    std::shared_ptr<Script> script;

    /* derived classes MUST implement GetEntityType() - not pure virtual due to dynamic casting constraints */
    virtual EntityType GetEntityType() { assert(false); return CBE_SPRITE; };

    bool HasScript() { return this->script != nullptr; };
    void MarkDestroy() { this->destroyed = true; };
    virtual void Render(SDL_Renderer *, const CB_ViewClippingInfo &){};
    virtual void Update(float){};
    virtual ~Entity(){};

  protected:
    Entity(){};

  private:
    bool destroyed = false;
};
}
#endif