#pragma once
#ifndef CBSDL_HPP
#define CBSDL_HPP

#include <SDL.h>
#include <utility>

#include "coord.hpp"

namespace Critterbits {
namespace SDLx {

void SDL_RenderTexture(SDL_Renderer *, SDL_Texture *, int, int);
void SDL_RenderTextureClipped(SDL_Renderer *, SDL_Texture *, const CB_Rect &, const CB_Rect &, bool = false,
                              bool = false, double = 0.);

// Cleanup functions
// Source: http://www.willusher.io/sdl2%20tutorials/2014/08/01/postscript-1-easy-cleanup

/*
 * Recurse through the list of arguments to clean up, cleaning up
 * the first one in the list each iteration.
 */
template <typename T, typename... Args>
void SDL_CleanUp(T * t, Args &&... args) {
    // Cleanup the first item in the list
    SDL_CleanUp(t);
    // Recurse to clean up the remaining arguments
    SDL_CleanUp(std::forward<Args>(args)...);
}

/*
 * These specializations serve to free the passed argument and also provide the
 * base cases for the recursive call above, eg. when args is only a single item
 * one of the specializations below will be called by
 * cleanup(std::forward<Args>(args)...), ending the recursion
 * We also make it safe to pass nullptrs to handle situations where we
 * don't want to bother finding out which values failed to load (and thus are null)
 * but rather just want to clean everything up and let cleanup sort it out
 */
template <>
inline void SDL_CleanUp<SDL_Window>(SDL_Window * window) {
    if (window == nullptr) {
        return;
    }
    SDL_DestroyWindow(window);
}

template <>
inline void SDL_CleanUp<SDL_Renderer>(SDL_Renderer * renderer) {
    if (renderer == nullptr) {
        return;
    }
    SDL_DestroyRenderer(renderer);
}

template <>
inline void SDL_CleanUp<SDL_Texture>(SDL_Texture * texture) {
    if (texture == nullptr) {
        return;
    }
    SDL_DestroyTexture(texture);
}

template <>
inline void SDL_CleanUp<SDL_Surface>(SDL_Surface * surface) {
    if (surface == nullptr) {
        return;
    }
    SDL_FreeSurface(surface);
}
}
}
#endif