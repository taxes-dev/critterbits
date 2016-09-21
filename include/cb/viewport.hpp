#pragma once
#ifndef CBVIEWPORT_HPP
#define CBVIEWPORT_HPP

#include <memory>

#include "coord.hpp"
#include "entity.hpp"

namespace Critterbits {

class Viewport : public Entity {
  public:
    Viewport(){};
    EntityType GetEntityType() const { return EntityType::Viewport; };
    CB_ViewClippingInfo GetStaticViewableRect(CB_Rect &, const ZIndex &) const;
    CB_ViewClippingInfo GetViewableRect(CB_Rect &, const ZIndex &) const;
    void SetEntityToFollow(std::shared_ptr<Entity>);

  protected:
    void OnUpdate(float);

  private:
    std::weak_ptr<Entity> entity_to_follow;
    CB_Point entity_center_view{0, 0};

    CB_ViewClippingInfo GetViewableRect(CB_Rect &, const ZIndex &, int, int) const;
};
}

#endif