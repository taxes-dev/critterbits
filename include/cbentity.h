#pragma once
#ifndef CBENTITY_H
#define CBENTITY_H

#include "cbcoord.h"
#include <SDL.h>

namespace Critterbits {
class Entity {
  public:
    CB_Rect dim = {0, 0, 0, 0};
    std::string tag = "";

    virtual void Render(SDL_Renderer *, const CB_ViewClippingInfo &){};
    virtual void Update(float){};
    virtual ~Entity(){};

  protected:
    Entity(){};
};
}
#endif