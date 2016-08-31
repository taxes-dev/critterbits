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

class Sprite : public Entity {
  public:
    std::string sprite_sheet_path = "";
    int tile_height = 0;
    int tile_width = 0;
    int tile_offset_x = 0;
    int tile_offset_y = 0;

    Sprite(){};
    ~Sprite();
    void NotifyLoaded();
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);

  private:
    int sprite_sheet_rows = 0;
    int sprite_sheet_cols = 0;
    SDL_Texture * sprite_sheet = nullptr;
};

class SpriteManager {
  public:
    std::list<std::shared_ptr<Sprite>> sprites;

    SpriteManager(){};
    bool LoadSprite(std::string &);
    void SetAssetPath(std::string &);

  private:
    std::string sprite_path;

    SpriteManager(const SpriteManager &) = delete;
    SpriteManager(SpriteManager &&) = delete;
};
}
#endif