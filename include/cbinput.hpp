#pragma once
#ifndef CBINPUT_HPP
#define CBINPUT_HPP

#include <map>
#include <vector>

#include <SDL.h>

namespace Critterbits {
typedef enum : int32_t { CBE_DIR_NONE = 0, CBE_DIR_LEFT = 1, CBE_DIR_RIGHT = 2, CBE_DIR_UP = 4, CBE_DIR_DOWN = 8 } CB_InputDirection;
typedef enum : int32_t { CBE_BTN_NONE = 0, CBE_BTN_BUTTON1 = 1, CBE_BTN_BUTTON2 = 2, CBE_BTN_BUTTON3 = 3, CBE_BUTTON_BTN4 = 4 } CB_ControllerButton;
typedef int32_t CB_KeyCode; // SDL_Keycode is an int32_t

class InputManager {
  public:
    InputManager(){};
    ~InputManager();

    void AddSdlEvent(const SDL_Event &);
    void CheckInputs();
    bool IsControllerAxisPressed(CB_InputDirection);
    bool IsKeyPressed(CB_KeyCode);
    void SetControllerActive(bool active);
    void SetKeyboardActive(bool active) { this->keyboard_active = active; };
    void SetMouseActive(bool active) { this->mouse_active = active; };

  private:
    SDL_GameController * controller{nullptr};
    std::map<SDL_Keycode, bool> keyboard_state;
    CB_InputDirection controller_axis_state{CBE_DIR_NONE};
    bool controller_active = false;
    bool keyboard_active = false;
    bool mouse_active = false;

    void AddNormalizedEvent();
    void CheckControllerInputs();
    void CheckMouseInputs();
    void InitializeController();

    InputManager(const InputManager &) = delete;
    InputManager(InputManager &&) = delete;
    void operator=(InputManager const &) = delete;
};
}
#endif