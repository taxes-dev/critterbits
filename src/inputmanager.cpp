#include <algorithm>

#include <critterbits.hpp>

namespace Critterbits {
InputManager::~InputManager() {
    if (this->controller != nullptr) {
        SDL_GameControllerClose(this->controller);
    }
}

void InputManager::AddSdlEvent(const SDL_Event & event) {
    if (this->keyboard_active && event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED) {
        // keyboard pressed
        auto state = this->keyboard_state.find(event.key.keysym.sym);
        if (state == this->keyboard_state.end()) {
            this->keyboard_state.insert(std::make_pair(event.key.keysym.sym, true));
        } else {
            state->second = true;
        }
    } else if (this->keyboard_active && event.type == SDL_KEYUP) {
        // keyboard released
        auto state = this->keyboard_state.find(event.key.keysym.sym);
        if (state != this->keyboard_state.end()) {
            state->second = false;
        }
    }
}

void InputManager::CheckInputs() {
    this->CheckControllerInputs();
    this->CheckMouseInputs();
    // keyboard inputs are handled by AddSdlEvent

    // check for normalized
    this->SetNormalizedInputs();
}

void InputManager::CheckControllerInputs() {
    if (this->controller_active) {
        SDL_GameControllerUpdate();

        InputDirection dpad{InputDirection::None};
        if (SDL_GameControllerGetButton(this->controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            SetBitMask(&dpad, InputDirection::Left);
        }
        if (SDL_GameControllerGetButton(this->controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            SetBitMask(&dpad, InputDirection::Right);
        }
        if (SDL_GameControllerGetButton(this->controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            SetBitMask(&dpad, InputDirection::Up);
        }
        if (SDL_GameControllerGetButton(this->controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            SetBitMask(&dpad, InputDirection::Down);
        }
        this->controller_axis_state = dpad;
    }
}

void InputManager::CheckMouseInputs() {
    if (this->mouse_active) {
        // TODO
    }
}

void InputManager::InitializeController() {
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        LOG_SDL_ERR("InputManager::InitializeController unable to initialze game controller subsystem");
        return;
    }

    // we're not using the events for joystick/controller
    SDL_JoystickEventState(SDL_IGNORE);
    SDL_GameControllerEventState(SDL_IGNORE);

    int controllers = 0;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controllers++;
            std::string name{SDL_GameControllerNameForIndex(i)};
            LOG_INFO("InputManager::InitializeController game controller " + std::to_string(i) + ": " +
                     (name.empty() ? "(unknown)" : name));
            this->controller = SDL_GameControllerOpen(i);
            if (this->controller != nullptr) {
                LOG_INFO("InputManager::InitializeController initialized");
                this->controller_active = true;
                break;
            }
        }
    }
    LOG_INFO("InputManager::InitializeController there are " + std::to_string(controllers) +
             " game controllers attached");
    if (this->controller == nullptr) {
        LOG_INFO("InputManager::InitializeController no suitable controllers found");
    }
}

bool InputManager::IsAxisPressed(const InputDirection & direction) {
    return TestBitMask(this->normal_axis_state, direction);
}

bool InputManager::IsControllerAxisPressed(const InputDirection & direction) {
    return TestBitMask(this->controller_axis_state, direction);
}

bool InputManager::IsKeyPressed(CB_KeyCode key_code) {
    auto state = this->keyboard_state.find(key_code);
    if (state != this->keyboard_state.end()) {
        return state->second;
    }
    return false;
}

void InputManager::SetControllerActive(bool active) {
    if (this->controller == nullptr && active) {
        this->InitializeController();
    } else {
        this->controller_active = active;
    }
}

void InputManager::SetNormalizedInputs() {
    this->normal_axis_state = InputDirection::None;
    if (this->controller_active) {
        this->normal_axis_state |= this->controller_axis_state;
    }
    if (this->keyboard_active) {
        if (this->IsKeyPressed(SDLK_LEFT)) {
            this->normal_axis_state |= InputDirection::Left;
        }
        if (this->IsKeyPressed(SDLK_RIGHT)) {
            this->normal_axis_state |= InputDirection::Right;
        }
        if (this->IsKeyPressed(SDLK_UP)) {
            this->normal_axis_state |= InputDirection::Up;
        }
        if (this->IsKeyPressed(SDLK_DOWN)) {
            this->normal_axis_state |= InputDirection::Down;
        }
    }
}
}