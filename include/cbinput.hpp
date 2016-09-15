#pragma once
#ifndef CBINPUT_HPP
#define CBINPUT_HPP

#include <map>
#include <vector>

#include <SDL.h>

namespace Critterbits {
typedef enum { CBE_INPUT_KEYBOARD } InputEventType;
typedef SDL_Keycode CB_KeyCode;

struct InputEvent {
    InputEventType type;
    CB_KeyCode key_code;

    InputEvent(InputEventType type) : type(type){};
};

class InputManager {
  public:
    InputManager(){};

    void AddSdlEvent(const SDL_Event &);
    void ClearInputEvents();
    void ContinueEvents();
    bool IsKeyPressed(CB_KeyCode);

  private:
    std::vector<InputEvent> events;
    std::map<SDL_Keycode, bool> keyboard_state;

    InputManager(const InputManager &) = delete;
    InputManager(InputManager &&) = delete;
    void operator=(InputManager const &) = delete;
};
}
#endif