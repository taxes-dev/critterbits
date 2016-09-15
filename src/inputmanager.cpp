#include <critterbits.hpp>

namespace Critterbits {
void InputManager::AddSdlEvent(const SDL_Event & event) {
    if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED) {
        // keyboard pressed event
        InputEvent kbd_event(CBE_INPUT_KEYBOARD);
        kbd_event.key_code = event.key.keysym.sym;
        this->events.push_back(kbd_event);
        auto cont_event = this->keyboard_state.find(event.key.keysym.sym);
        if (cont_event == this->keyboard_state.end()) {
            this->keyboard_state.insert(std::make_pair(event.key.keysym.sym, true));
        } else {
            cont_event->second = true;
        }
    } else if (event.type == SDL_KEYUP) {
        // keyboard released
        auto cont_event = this->keyboard_state.find(event.key.keysym.sym);
        if (cont_event != this->keyboard_state.end()) {
            cont_event->second = false;
        }
    }
}

void InputManager::ClearInputEvents() { this->events.clear(); }

void InputManager::ContinueEvents() {
    for (auto & cont_event : this->keyboard_state) {
        if (cont_event.second) {
            InputEvent kbd_event(CBE_INPUT_KEYBOARD);
            kbd_event.key_code = cont_event.first;
            this->events.push_back(kbd_event);
        }
    }
}

bool InputManager::IsKeyPressed(CB_KeyCode key_code) {
    for (auto & event : this->events) {
        if (event.type == CBE_INPUT_KEYBOARD && event.key_code == key_code) {
            return true;
        }
    }
    return false;
}
}