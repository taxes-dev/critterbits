#pragma once
#ifndef CBENTITY_H
#define CBENTITY_H

#include "cbcoord.h"
#include <SDL.h>

namespace Critterbits {
class Entity {
  public:
    CB_Rect dim;
    virtual void Render(SDL_Renderer *, const CB_ViewClippingInfo &){};
    virtual ~Entity(){};

  protected:
    Entity(){};
};
}
#endif