#include <SDL.h>

#include <cbsdl.h>

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

void SDL_RenderTextureClipped(SDL_Renderer * renderer, SDL_Texture * texture, const CB_Rect & source,
                              const CB_Rect & dest, bool flip_x, bool flip_y) {
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect src;
    src.x = source.x;
    src.y = source.y;
    src.w = source.w;
    src.h = source.h;
    SDL_Rect dst;
    dst.x = dest.x;
    dst.y = dest.y;
    dst.w = dest.w;
    dst.h = dest.h;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flip_x) {
        flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    }
    if (flip_y) {
        flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
    }
    SDL_RenderCopyEx(renderer, texture, &src, &dst, 0., NULL, flip);
}
}
}