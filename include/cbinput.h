#pragma once
#ifndef CBINPUT_H
#define CBINPUT_H

#include <vector>

#include <SDL.h>

namespace Critterbits {
typedef enum { CBE_INPUT_KEYBOARD } InputEventType;

struct InputEvent {
    InputEventType type;
    int key_code = 0;

    InputEvent(InputEventType type) : type(type){};
};

class InputManager {
  public:
    InputManager(){};

    void AddSdlEvent(const SDL_Event &);
    void ClearInputEvents();
    bool IsKeyPressed(int);

  private:
    std::vector<InputEvent> events;

    InputManager(const InputManager &) = delete;
    InputManager(InputManager &&) = delete;
    void operator=(InputManager const &) = delete;
};
}
#endif