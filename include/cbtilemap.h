#pragma once
#ifndef CBTILEMAP_H
#define CBTILEMAP_H

#include <SDL.h>
#include <tmx/tmx.h>

#include "cbcoord.h"
#include "cbentity.h"

namespace Critterbits {

class Tilemap : public Entity {
  public:
    int tile_width;
    int tile_height;

    Tilemap(const std::string &);
    ~Tilemap();
    bool CreateTextures(float scale);
    SDL_Texture * GetMapTexture() const { return this->map_texture; };
    void Render(SDL_Renderer *, const CB_ViewClippingInfo &);
    static void Tilemap_Init();
    static void Tilemap_Quit();

  private:
    std::string tmx_path;
    tmx_map * map = nullptr;
    SDL_Texture * map_texture = nullptr;
    bool draw_debug = false;

    SDL_Texture * RenderMap(SDL_Renderer *, float);
    void DrawImageLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *);
    void DrawMapLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *);
    void DrawObjectLayer(SDL_Renderer *, SDL_Texture *, const tmx_layer *);
    inline void DrawTileOnMap(SDL_Renderer *, unsigned int, int, int, int, int, int);
};
}
#endif