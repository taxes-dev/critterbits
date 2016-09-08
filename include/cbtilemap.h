#pragma once
#ifndef CBTILEMAP_H
#define CBTILEMAP_H

#include <SDL.h>
#include <tmx/tmx.h>

#include "cbcoord.h"
#include "cbentity.h"
#include "cb2d.h"
#include "cbsprite.h"

#define CB_TILEMAP_COLLIDE "collide"
#define CB_TILEMAP_FOREGROUND "foreground"

#define CB_TILEMAP_TMX_PROP_BOOL_TRUE "true"

namespace Critterbits {
class TilemapRegion : public Sprite {
public:
  TilemapRegion();

  void Render(SDL_Renderer *, const CB_ViewClippingInfo &);

private:
  bool draw_debug{false};
  
};

class Tilemap : public Entity {
  public:
    int tile_width;
    int tile_height;
    std::vector<std::shared_ptr<TilemapRegion>> regions;

    Tilemap(const std::string &);
    ~Tilemap();
    bool CreateTextures(float scale);
    EntityType GetEntityType() { return CBE_TILEMAP; };
    SDL_Texture * GetMapTexture() const { return this->map_texture; };
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);
    static void Tilemap_Init();
    static void Tilemap_Quit();

  private:
    struct MapTile {
      unsigned int gid;
      int row;
      int col;
      int offsetx;
      int offsety;
      int alpha_mod;
    };
    std::string tmx_path;
    tmx_map * map{nullptr};
    SDL_Texture * map_texture{nullptr};
    bool draw_debug{false};
    float render_scale{1.0f};

    SDL_Texture * RenderMap(SDL_Renderer *, float);
    void CreateCollisionRegion(const CB_Rect &);
    void DrawImageLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *);
    void DrawMapLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *, RectRegionCombiner &);
    void DrawObjectLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *);
    inline void DrawTileOnMap(SDL_Renderer *, const struct MapTile &, RectRegionCombiner * = nullptr);
};
}
#endif