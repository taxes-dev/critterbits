#pragma once
#ifndef CBBOXCOLLIDER_HPP
#define CBBOXCOLLIDER_HPP

#include <vector>

#include "coord.hpp"
#include "entity.hpp"

namespace Critterbits {
enum class CollisionType { None, Collide, Trigger };

class BoxCollider : public Entity {
  public:
    CollisionType collision{CollisionType::None};
    CB_Rect collision_box;

    BoxCollider(){};
    ~BoxCollider(){};
    CB_Rect GetCollisionRect() const;
    CB_Point GetValidPosition(int, int, int, int);
    virtual void SetPosition(int, int) = 0;

  private:
    std::vector<entity_id_t> is_colliding_with;

    bool IsCollidingWith(entity_id_t);
    void NotifyCollision(std::weak_ptr<BoxCollider>);
    void RemoveCollisionWith(entity_id_t);
};
}
#endif