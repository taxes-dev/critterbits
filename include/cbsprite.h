#pragma once
#ifndef CBSPRITE_H
#define CBSPRITE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>

#include "cbentity.h"

#define CB_SPRITE_PATH "sprites"
#define CB_SPRITE_EXT ".toml"

namespace Critterbits {
typedef enum { CBE_SPRITE_NEW, CBE_SPRITE_READY, CBE_SPRITE_UNLOADED } SpriteState;
typedef enum { CBE_COLLIDE_NONE, CBE_COLLIDE_COLLIDE, CBE_COLLIDE_TRIGGER } CollisionType;

typedef struct QueuedSprite {
  std::string name;
  CB_Point at;
} QueuedSprite;

class Sprite : public Entity {
  public:
    SpriteState state{CBE_SPRITE_NEW};
    std::string sprite_name;
    std::string sprite_path;
    std::string sprite_sheet_path;
    float sprite_scale{1.0f};
    int tile_height{0};
    int tile_width{0};
    int tile_offset_x{0};
    int tile_offset_y{0};
    bool flip_x{false};
    bool flip_y{false};
    CollisionType collision{CBE_COLLIDE_NONE};

    Sprite();
    ~Sprite();
    EntityType GetEntityType() { return CBE_SPRITE; };
    inline int GetFrame() const { return this->current_frame; };
    inline int GetFrameCount() const { return this->sprite_sheet_rows * this->sprite_sheet_cols; }
    void NotifyCollision(std::weak_ptr<Sprite>);
    void NotifyLoaded();
    void NotifyUnloaded();
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);
    void SetFrame(int);
    void SetPosition(int, int);

  protected:
    bool OnStart();

  private:
    int current_frame{0};
    int sprite_sheet_rows{0};
    int sprite_sheet_cols{0};
    bool draw_debug{false};
    std::shared_ptr<SDL_Texture> sprite_sheet;
    bool sprite_sheet_loaded{false};
    bool script_loaded{false};
    std::vector<entity_id_t> is_colliding_with;

    CB_Rect GetFrameRect() const;
    bool IsCollidingWith(entity_id_t);
    void RemoveCollisionWith(entity_id_t);
};

class SpriteManager {
  public:
    std::vector<std::shared_ptr<Sprite>> sprites;

    SpriteManager(){};
    std::string GetSpritePath(const std::string &);
    std::shared_ptr<SDL_Texture> GetSpriteSheet(const std::string &);
    bool LoadQueuedSprites();
    void QueueSprite(const QueuedSprite &);
    void UnloadSprite(std::shared_ptr<Sprite>);

  private:
    std::vector<QueuedSprite> queued_sprites;
    std::map<std::string, std::shared_ptr<SDL_Texture>> sprite_sheets;

    SpriteManager(const SpriteManager &) = delete;
    SpriteManager(SpriteManager &&) = delete;
};
}
#endif