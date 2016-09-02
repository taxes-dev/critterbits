#pragma once
#ifndef CBVIEWPORT_H
#define CBVIEWPORT_H

#include <memory>

#include "cbcoord.h"
#include "cbentity.h"

namespace Critterbits {

class Viewport : public Entity {
  public:
    Viewport(){};
    CB_ViewClippingInfo GetViewableRect(CB_Rect &) const;
    void SetEntityToFollow(const std::shared_ptr<Entity> &);
    void Update(float);

  private:
    std::weak_ptr<Entity> entity_to_follow;
    CB_Point entity_center_view;
};
}

#endif