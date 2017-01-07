#pragma once
#ifndef CBSPRITE_HPP
#define CBSPRITE_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>

#include "entity.hpp"
#include "boxcollider.hpp"
#include "anim.hpp"
#include "toml.hpp"

#define CB_SPRITE_PATH "sprites"
#define CB_SPRITE_EXT ".toml"

namespace Critterbits {

typedef struct QueuedSprite {
  std::string name;
  CB_Point at;
} QueuedSprite;

class Sprite : public BoxCollider {
  public:
    std::string sprite_name;
    std::string sprite_path;
    std::string sprite_sheet_path;
    std::string script_path;
    float sprite_scale{1.0f};
    int tile_height{0};
    int tile_width{0};
    int tile_offset_x{0};
    int tile_offset_y{0};
    bool flip_x{false};
    bool flip_y{false};
    CB_Color tint_and_opacity{255,255,255,255};
    std::vector<std::shared_ptr<Animation::Animation>> animations;

    Sprite();
    ~Sprite();
    EntityType GetEntityType() const { return EntityType::Sprite; };
    inline int GetFrame() const { return this->current_frame; };
    inline int GetFrameCount() const { return this->sprite_sheet_rows * this->sprite_sheet_cols; }
    void NotifyLoaded();
    void NotifyUnloaded();
    void SetFrame(int);
    void SetPosition(int, int);

  protected:
    bool OnStart();
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
    void OnUpdate(float);

  private:
    int current_frame{0};
    int sprite_sheet_rows{0};
    int sprite_sheet_cols{0};
    std::shared_ptr<SDL_Texture> sprite_sheet;
    bool sprite_sheet_loaded{false};
    bool script_loaded{false};

    CB_Rect GetFrameRect() const;
};

class SpriteManager {
  public:
    std::vector<std::shared_ptr<Sprite>> sprites;

    SpriteManager(){};
    bool LoadQueuedSprites();
    void QueueSprite(const QueuedSprite &);
    void UnloadSprite(std::shared_ptr<Sprite>);

  private:
    std::vector<QueuedSprite> queued_sprites;
    bool new_sprites{false};

    SpriteManager(const SpriteManager &) = delete;
    SpriteManager(SpriteManager &&) = delete;
    std::string GetSpritePath(const std::string &) const;
    std::string GetSpriteSheetPath(const std::string &) const;
    void ParseSprite(const Toml::TomlParser &, std::shared_ptr<Sprite>) const;
};
}
#endif