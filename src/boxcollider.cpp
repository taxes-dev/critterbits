#include <cb/critterbits.hpp>

namespace Critterbits {
bool BoxCollider::IsCollidingWith(entity_id_t entity_id) {
    for (auto & eid : this->is_colliding_with) {
        if (eid == entity_id) {
            return true;
        }
    }
    return false;
}

CB_Rect BoxCollider::GetCollisionRect() const {
    return CB_Rect{this->dim.x + this->collision_box.x, this->dim.y + this->collision_box.y, this->collision_box.w, this->collision_box.h};
}

CB_Point BoxCollider::GetValidPosition(int old_x, int old_y, int new_x, int new_y) {
    // check collisions at new position if we collide
    if (this->collision == CollisionType::Collide) {
        CB_Rect new_dim{this->GetCollisionRect()};
        new_dim.x = new_x;
        new_dim.y = new_y;

        do {
            // these get reset on each loop to check if we got moved by collision
            new_x = new_dim.x;
            new_y = new_dim.y;

            Engine::GetInstance().IterateActiveColliders([&](std::shared_ptr<BoxCollider> collider) {
                if (collider->entity_id != this->entity_id) {
                    if (collider->collision == CollisionType::Collide || collider->collision == CollisionType::Trigger) {
                        // check for collision
                        CB_Rect coll_box{collider->GetCollisionRect()};
                        if (AabbCollision(new_dim, coll_box)) {
                            // if full collision, adjust new x/y so they're not inside the collided sprite
                            if (collider->collision == CollisionType::Collide) {
                                if (new_dim.x > old_x) {
                                    new_dim.x = std::max(old_x, coll_box.x - new_dim.w);
                                } else if (new_dim.x < old_x) {
                                    new_dim.x = std::min(old_x, coll_box.right());
                                }
                                if (new_dim.y > old_y) {
                                    new_dim.y = std::max(old_y, coll_box.y - new_dim.h);
                                } else if (new_dim.y < old_y) {
                                    new_dim.y = std::min(old_y, coll_box.bottom());
                                }
                            }

                            // notify both sprites that a collision occurred
                            this->NotifyCollision(collider);
                            collider->NotifyCollision(std::dynamic_pointer_cast<BoxCollider>(this->shared_from_this()));
                        } else {
                            this->RemoveCollisionWith(collider->entity_id);
                        }
                    }
                }
                return false;
            });
        } while (new_dim.x != new_x || new_dim.y != new_y);
    }
    return CB_Point{new_x, new_y};
}

void BoxCollider::NotifyCollision(std::weak_ptr<BoxCollider> other) {
    if (auto oth = other.lock()) {
        if (oth->IsActive() && !this->IsCollidingWith(oth->entity_id)) {
            // record the collision (this is used for de-dupe)
            this->is_colliding_with.push_back(oth->entity_id);

            // queue up an event for the collision
            std::weak_ptr<BoxCollider> current = std::dynamic_pointer_cast<BoxCollider>(this->shared_from_this());
            EngineEventQueue::GetInstance().QueueCollision([other, current]() {
                if (auto cent = current.lock()) {
                    if (auto oent = other.lock()) {
                        // call oncollision script if it exists
                        if (cent->HasScript()) {
                            cent->script->CallOnCollision(cent, oent);
                        }

                        // full colliders reset after every hit, triggers will collide only once until the colliding
                        // object leaves the trigger area
                        if (cent->collision == CollisionType::Collide && oent->collision != CollisionType::Trigger) {
                            cent->RemoveCollisionWith(oent->entity_id);
                        }
                    }
                }
            });
        }
    }
}

void BoxCollider::RemoveCollisionWith(entity_id_t entity_id) {
    for (auto it = this->is_colliding_with.begin(); it != this->is_colliding_with.end(); it++) {
        if (*it == entity_id) {
            this->is_colliding_with.erase(it);
            break;
        }
    }
}
}