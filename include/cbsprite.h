#pragma once
#ifndef CBSPRITE_H
#define CBSPRITE_H

#include <list>
#include <memory>
#include <string>

#include "cbentity.h"
#include <SDL.h>

#define CB_SPRITE_PATH "sprites"
#define CB_SPRITE_EXT ".yml"

namespace Critterbits {
extern bool cb_draw_debug_sprite_rects;

class Sprite : public Entity {
  public:
    std::string sprite_path;
    std::string sprite_sheet_path = "";
    float sprite_scale = 1.0f;
    int tile_height = 0;
    int tile_width = 0;
    int tile_offset_x = 0;
    int tile_offset_y = 0;
    bool flip_x = false;
    bool flip_y = false;

    Sprite(){};
    ~Sprite();
    inline int GetFrame() const { return this->current_frame; };
    inline int GetFrameCount() const { return this->sprite_sheet_rows * this->sprite_sheet_cols; }
    void NotifyLoaded();
    void NotifyUnloaded();
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);
    void SetFrame(int);

  private:
    int current_frame = 0;
    int sprite_sheet_rows = 0;
    int sprite_sheet_cols = 0;
    SDL_Texture * sprite_sheet = nullptr; // TODO: centrally manage sprite sheets so they can be shared between sprites

    CB_Rect GetFrameRect() const;
};

class SpriteManager {
  public:
    std::list<std::shared_ptr<Sprite>> sprites;

    SpriteManager(){};
    bool LoadQueuedSprites();
    void QueueSprite(std::string &);
    void SetAssetPath(std::string &);

  private:
    std::string sprite_path;
    std::list<std::string> queued_sprites;

    SpriteManager(const SpriteManager &) = delete;
    SpriteManager(SpriteManager &&) = delete;
};
}
#endif