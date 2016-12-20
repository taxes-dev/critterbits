#pragma once
#ifndef CBTILEMAP_HPP
#define CBTILEMAP_HPP

#include <SDL.h>
#include <TmxMap.h>
#include <TmxMapTile.h>
#include <map>
#include <memory>
#include <string>

#include "2d.hpp"
#include "coord.hpp"
#include "entity.hpp"
#include "sprite.hpp"

#define CB_TILEMAP_COLLIDE "collide"
#define CB_TILEMAP_FOREGROUND "foreground"

#define CB_TILEMAP_TMX_PROP_BOOL_TRUE "true"

namespace Critterbits {
class TilemapRegion : public BoxCollider {
  public:
    TilemapRegion();

    EntityType GetEntityType() const { return EntityType::TilemapRegion; };
    void SetPosition(int, int){}; // map regions are static

  protected:
    bool OnStart() { return true; }
    void OnDebugRender(SDL_Renderer *, const CB_ViewClippingInfo &);
};

class Tilemap : public Entity {
  public:
    int tile_width;
    int tile_height;
    std::vector<std::shared_ptr<TilemapRegion>> regions;
    SDL_Color bg_color{0, 0, 0, 0};

    Tilemap(const std::string &);
    ~Tilemap();
    bool CreateTextures(float scale);
    EntityType GetEntityType() const { return EntityType::Tilemap; };
  
  protected:
    void OnRender(SDL_Renderer *, const CB_ViewClippingInfo &);

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
    void DrawImageLayer(SDL_Renderer *, const Tmx::ImageLayer *);
    void DrawMapLayer(SDL_Renderer *, const Tmx::TileLayer *, RectRegionCombiner *);
    void DrawObjectLayer(SDL_Renderer *, const Tmx::ObjectGroup *);
    inline void DrawTileOnMap(SDL_Renderer *, const Tmx::MapTile &, const MapTileInfo &,
                              RectRegionCombiner * = nullptr);
};

class TilesetImageManager {
  public:
    static TilesetImageManager & GetInstance();

    std::shared_ptr<SDL_Texture> GetTilesetImage(const std::string &, const std::string &);

  private:
    std::map<std::string, std::shared_ptr<SDL_Texture>> tileset_images;

    TilesetImageManager(){};
    TilesetImageManager(const TilesetImageManager &) = delete;
    TilesetImageManager(TilesetImageManager &&) = delete;
};
}
#endif