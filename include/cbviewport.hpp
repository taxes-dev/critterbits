#pragma once
#ifndef CBVIEWPORT_HPP
#define CBVIEWPORT_HPP

#include <memory>

#include "cbcoord.hpp"
#include "cbentity.hpp"

namespace Critterbits {

class Viewport : public Entity {
  public:
    Viewport(){};
    EntityType GetEntityType() { return CBE_VIEWPORT; };
    CB_ViewClippingInfo GetViewableRect(CB_Rect &) const;
    void SetEntityToFollow(std::shared_ptr<Entity>);

  protected:
    void OnUpdate(float);

  private:
    std::weak_ptr<Entity> entity_to_follow;
    CB_Point entity_center_view{0, 0};
};
}

#endif