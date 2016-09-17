#pragma once
#ifndef CBINPUT_HPP
#define CBINPUT_HPP

#include <map>
#include <vector>

#include <SDL.h>

#include "cbmath.hpp"

namespace Critterbits {
enum class InputDirection { None = 0, Left = 1, Right = 2, Up = 4, Down = 8 };
enum class InputButton { None = 0, Primary = 1, Secondary = 2, Cancel = 4, Accept = 8 };
enum class InputControllerButton { None = 0, Button1 = 1, Button2 = 2, Button3 = 4, Button4 = 8 };
typedef SDL_Keycode CB_KeyCode;

class InputManager {
  public:
    InputManager() {};
    ~InputManager();

    void AddSdlEvent(const SDL_Event &);
    void CheckInputs();
    bool IsAxisPressed(const InputDirection &);
    bool IsControllerAxisPressed(const InputDirection &);
    bool IsKeyPressed(CB_KeyCode);
    void SetControllerActive(bool active);
    void SetKeyboardActive(bool active) { this->keyboard_active = active; };
    void SetMouseActive(bool active) { this->mouse_active = active; };

  private:
    SDL_GameController * controller{nullptr};
    std::map<SDL_Keycode, bool> keyboard_state;
    InputButton normal_button_state{InputButton::None};
    InputDirection controller_axis_state{InputDirection::None};
    InputDirection normal_axis_state{InputDirection::None};
    bool controller_active = false;
    bool keyboard_active = false;
    bool mouse_active = false;

    void CheckControllerInputs();
    void CheckMouseInputs();
    void InitializeController();
    void SetNormalizedInputs();

    InputManager(const InputManager &) = delete;
    InputManager(InputManager &&) = delete;
    void operator=(InputManager const &) = delete;
};
}
#endif