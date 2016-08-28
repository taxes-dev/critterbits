#pragma once
#ifndef CBENGINE_H
#define CBENGINE_H

#include <SDL.h>

#define CB_DEFAULT_WINDOW_W 800
#define CB_DEFAULT_WINDOW_H 600

namespace Critterbits {

class Engine {
  public:
    int window_width = CB_DEFAULT_WINDOW_W;
    int window_height = CB_DEFAULT_WINDOW_H;
    SDL_Rect display_bounds;

    Engine(){};
    ~Engine();
    int Run();

  private:
    SDL_Window * window = nullptr;
    SDL_Renderer * renderer = nullptr;

    Engine(const Engine &) = delete;
    Engine(Engine &&) = delete;
};
}
#endif