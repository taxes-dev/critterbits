#include <SDL.h>

#include "cblogging.h"
#include "cbsdl.h"

namespace Critterbits {
namespace SDLx {
void SDL_RenderTexture(SDL_Renderer * renderer, SDL_Texture * texture, int x, int y) {
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    // Setup the destination rectangle to be at the position we want
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

void SDL_RenderTextureClipped(SDL_Renderer * renderer, SDL_Texture * texture, int x, int y, int offset_x,
                              int offset_y) {
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect src;
    src.x = offset_x;
    src.y = offset_y;
    src.w = w - offset_x;
    src.h = h - offset_y;
    LOG_INFO("Got w/h " + std::to_string(src.w) + " " + std::to_string(src.h));
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = src.w;
    dst.h = src.h;
    SDL_RenderCopy(renderer, texture, &src, &dst);
}
}
}