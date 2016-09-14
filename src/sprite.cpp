#include <algorithm>

#include <critterbits.hpp>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>

namespace Critterbits {

Sprite::Sprite() { this->draw_debug = Engine::GetInstance().config->debug.draw_sprite_rects; }

Sprite::~Sprite() {}

CB_Rect Sprite::GetFrameRect() const {
    CB_Rect frame_rect;
    frame_rect.x = this->tile_offset_x + this->tile_width * (this->current_frame % this->sprite_sheet_rows);
    frame_rect.y = this->tile_offset_y + this->tile_height * (this->current_frame / this->sprite_sheet_cols);
    frame_rect.w = this->tile_width;
    frame_rect.h = this->tile_height;
    return frame_rect;
}

bool Sprite::IsCollidingWith(entity_id_t entity_id) {
    for (auto & eid : this->is_colliding_with) {
        if (eid == entity_id) {
            return true;
        }
    }
    return false;
}

void Sprite::NotifyCollision(std::weak_ptr<Sprite> other_sprite) {
    if (auto spr = other_sprite.lock()) {
        if (!this->IsCollidingWith(spr->entity_id)) {
            // record the collision (this is used for de-dupe)
            this->is_colliding_with.push_back(spr->entity_id);

            // queue up an event for the collision
            std::weak_ptr<Sprite> this_sprite = std::dynamic_pointer_cast<Sprite>(shared_from_this());
            EngineEventQueue::GetInstance().QueueCollision([other_sprite, this_sprite]() {
                if (auto tspr = this_sprite.lock()) {
                    if (auto ospr = other_sprite.lock()) {
                        // call oncollision script if it exists
                        if (tspr->HasScript()) {
                            tspr->script->CallOnCollision(tspr, ospr);
                        }

                        // full colliders reset after every hit, triggers will collide only once until the colliding
                        // object leaves the trigger area
                        if (tspr->collision == CBE_COLLIDE_COLLIDE && ospr->collision != CBE_COLLIDE_TRIGGER) {
                            tspr->RemoveCollisionWith(ospr->entity_id);
                        }
                    }
                }
            });
        }
    }
}

void Sprite::NotifyLoaded() {
    LOG_INFO("Sprite::NotifyLoaded sprite was loaded " + this->sprite_name);
    this->dim.w = this->tile_width * this->sprite_scale;
    this->dim.h = this->tile_height * this->sprite_scale;

    EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
        LOG_INFO("Sprite::NotifyLoaded(pre-update) attempting to load sprite sheet " + this->sprite_sheet_path);
        this->sprite_sheet =
            Engine::GetInstance().scenes.current_scene->sprites.GetSpriteSheet(this->sprite_sheet_path);
        if (this->sprite_sheet == nullptr) {
            LOG_ERR("Sprite::NotifyLoaded(pre-update) unable to load sprite sheet");
        } else {
            int w, h;
            SDL_QueryTexture(this->sprite_sheet.get(), NULL, NULL, &w, &h);
            this->sprite_sheet_cols = (w - this->tile_offset_x) / this->tile_width;
            this->sprite_sheet_rows = (h - this->tile_offset_y) / this->tile_height;
        }
        this->sprite_sheet_loaded = true;
    });

    EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
        LOG_INFO("Sprite::NotifyLoaded(pre-update) attempting to load sprite script " + this->sprite_name);

        this->script = Engine::GetInstance().scripts.LoadScript(this->sprite_name);
        this->script_loaded = true;
    });
}

void Sprite::NotifyUnloaded() {
    LOG_INFO("Sprite::NotifyUnloaded sprite was unloaded " + this->sprite_name);
    this->state = CBE_SPRITE_UNLOADED;
}

void Sprite::RemoveCollisionWith(entity_id_t entity_id) {
    for (auto it = this->is_colliding_with.begin(); it != this->is_colliding_with.end(); it++) {
        if (*it == entity_id) {
            this->is_colliding_with.erase(it);
            break;
        }
    }
}

void Sprite::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    Entity::Render(renderer, clip_rect);
    if (this->state == CBE_SPRITE_READY) {
        if (this->sprite_sheet != nullptr && clip_rect.z_index == CBE_Z_MIDGROUND) {
            // FIXME: hack to prevent sprites from getting squished (GetFrameRect() needs to adjust for clipping)
            CB_Rect dst_rect = clip_rect.dest;
            dst_rect.x -= clip_rect.source.x;
            dst_rect.y -= clip_rect.source.y;
            dst_rect.w = this->dim.w;
            dst_rect.h = this->dim.h;
            SDLx::SDL_RenderTextureClipped(renderer, this->sprite_sheet.get(), this->GetFrameRect(), dst_rect,
                                           this->flip_x, this->flip_y);
        }
        if (this->draw_debug && clip_rect.z_index == CBE_Z_FOREGROUND) {
            rectangleRGBA(renderer, clip_rect.dest.x, clip_rect.dest.y, clip_rect.dest.right(), clip_rect.dest.bottom(),
                          255, 0, 0, 127);
            boxRGBA(renderer, clip_rect.dest.x, clip_rect.dest.bottom(), clip_rect.dest.x + this->tag.length() * 8 + 2,
                    clip_rect.dest.bottom() + 10, 255, 0, 0, 127);
            stringRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.bottom() + 1, this->tag.c_str(), 255, 255, 255,
                       255);
            std::string coords = this->dim.xy().to_string();
            boxRGBA(renderer, clip_rect.dest.right() - 8 * coords.length() - 2, clip_rect.dest.y - 10,
                    clip_rect.dest.right() - 1, clip_rect.dest.y - 1, 255, 0, 0, 127);
            stringRGBA(renderer, clip_rect.dest.right() - 8 * coords.length() - 1, clip_rect.dest.y - 9, coords.c_str(),
                       255, 255, 255, 255);
            std::string f = std::to_string(this->current_frame);
            boxRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.y + 1, clip_rect.dest.x + f.length() * 8 + 2,
                    clip_rect.dest.y + 10, 255, 0, 0, 127);
            stringRGBA(renderer, clip_rect.dest.x + 1, clip_rect.dest.y + 1, f.c_str(), 255, 255, 255, 255);
        }
    }
}

void Sprite::SetFrame(int frame) { this->current_frame = Clamp(frame, 0, this->GetFrameCount()); }

void Sprite::SetPosition(int new_x, int new_y) {
    if (new_x == this->dim.x && new_y == this->dim.y) {
        return;
    }

    // check collisions at new position if we collide
    if (this->collision == CBE_COLLIDE_COLLIDE) {
        CB_Rect new_dim{new_x, new_y, this->dim.w, this->dim.h};
        Engine::GetInstance().IterateActiveEntities([&](std::shared_ptr<Entity> entity) {
            if (entity->GetEntityType() == CBE_SPRITE && entity->entity_id != this->entity_id) {
                std::shared_ptr<Sprite> sprite = std::dynamic_pointer_cast<Sprite>(entity);
                if (sprite->collision == CBE_COLLIDE_COLLIDE || sprite->collision == CBE_COLLIDE_TRIGGER) {
                    // check for collision
                    if (AabbCollision(new_dim, sprite->dim)) {
                        // if full collision, adjust new x/y so they're not inside the collided sprite
                        if (sprite->collision == CBE_COLLIDE_COLLIDE) {
                            if (new_x > this->dim.x) {
                                new_x = std::max(this->dim.x, sprite->dim.x - this->dim.w);
                            } else if (new_x < this->dim.x) {
                                new_x = std::min(this->dim.x, sprite->dim.right());
                            }
                            if (new_y > this->dim.y) {
                                new_y = std::max(this->dim.y, sprite->dim.y - this->dim.h);
                            } else if (new_y < this->dim.y) {
                                new_y = std::min(this->dim.y, sprite->dim.bottom());
                            }
                        }

                        // notify both sprites that a collision occurred
                        this->NotifyCollision(sprite);
                        sprite->NotifyCollision(std::dynamic_pointer_cast<Sprite>(shared_from_this()));
                    } else {
                        this->RemoveCollisionWith(sprite->entity_id);
                    }
                }
            }
            return false;
        });
    }

    // update position if valid
    this->dim.x = new_x;
    this->dim.y = new_y;
}

bool Sprite::OnStart() {
    // delay start until resources loaded
    if (this->sprite_sheet_loaded && this->script_loaded) {
        this->state = CBE_SPRITE_READY;
        return true;
    }
    return false;
}
}