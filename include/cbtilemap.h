#pragma once
#ifndef CBTILEMAP_H
#define CBTILEMAP_H

#include <tmx/tmx.h>

namespace Critterbits {
class Tilemap {
  public:
    Tilemap(std::string & map_path) : tmx_path(map_path){};
    ~Tilemap();
    static void Tilemap_Init();
    bool CreateTextures();
    SDL_Texture * GetMapTexture() const { return this->map_texture; };

  private:
    std::string tmx_path;
    tmx_map * map = nullptr;
    SDL_Rect map_rect;
    SDL_Texture * map_texture = nullptr;

    SDL_Texture * RenderMap();
};
}
#endif