#pragma once
#ifndef CBTILEMAP_H
#define CBTILEMAP_H

#include <map>
#include <memory>
#include <string>
#include <SDL.h>
#include <TmxMap.h>
#include <TmxMapTile.h>

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
  void SetPosition(int new_x, int new_y) {}; // map regions are static

protected:
  bool OnStart() { return true; }
  
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
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);

  private:
    struct MapTileInfo {
      int row;
      int col;
      int offsetx;
      int offsety;
      int alpha_mod;
    };
    std::string tmx_path;
    std::unique_ptr<Tmx::Map> map{nullptr};
    SDL_Texture * bg_map_texture{nullptr};
    SDL_Texture * fg_map_texture{nullptr};
    bool draw_debug{false};
    float render_scale{1.0f};

    bool RenderMap(SDL_Renderer *, float);
    void CreateCollisionRegion(const CB_Rect &);
    void DrawImageLayer(SDL_Renderer *, SDL_Texture *, const Tmx::ImageLayer *);
    void DrawMapLayer(SDL_Renderer *, SDL_Texture *, const Tmx::TileLayer *, RectRegionCombiner *);
    void DrawObjectLayer(SDL_Renderer *, SDL_Texture *, const Tmx::ObjectGroup *);
    inline void DrawTileOnMap(SDL_Renderer *, const Tmx::MapTile &, const MapTileInfo &, RectRegionCombiner * = nullptr);
};

class TilesetImageManager {
  public:
    static TilesetImageManager & GetInstance();

    std::shared_ptr<SDL_Texture> GetTilesetImage(const std::string &, const std::string &);

private:
  std::map<std::string, std::shared_ptr<SDL_Texture>> tileset_images;

    TilesetImageManager() {};
    TilesetImageManager(const TilesetImageManager &) = delete;
    TilesetImageManager(TilesetImageManager &&) = delete;
};
}
#endif