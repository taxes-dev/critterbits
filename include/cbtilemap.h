#pragma once
#ifndef CBTILEMAP_H
#define CBTILEMAP_H

#include <tmx/tmx.h>

#include "cbcoord.h"

namespace Critterbits {
extern bool cb_force_draw_map_regions;

class Tilemap {
  public:
    CB_Rect map_rect;
    int tile_width;
    int tile_height;

    Tilemap(std::string & map_path) : tmx_path(map_path){};
    ~Tilemap();
    bool CreateTextures(float scale);
    SDL_Texture * GetMapTexture() const { return this->map_texture; };
    static void Tilemap_Init(bool);

  private:
    std::string tmx_path;
    tmx_map * map = nullptr;
    SDL_Texture * map_texture = nullptr;

    SDL_Texture * RenderMap(SDL_Renderer *, float) const;
    void DrawImageLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *) const;
    void DrawMapLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *) const;
    void DrawObjectLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *) const;
};
}
#endif