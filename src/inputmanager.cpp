#include <SDL.h>

#include <critterbits.h>

namespace Critterbits {
void InputManager::AddSdlEvent(const SDL_Event & event) {
    if (event.type == SDL_KEYDOWN && event.key.state == SDL_PRESSED) {
        // keyboard pressed event
        InputEvent kbd_event(CBE_INPUT_KEYBOARD);
        kbd_event.key_code = event.key.keysym.sym;
        this->events.push_back(kbd_event);
    }
}

void InputManager::ClearInputEvents() { this->events.clear(); }

bool InputManager::IsKeyPressed(int key_code) {
    for (auto & event : this->events) {
        if (event.type == CBE_INPUT_KEYBOARD && event.key_code == key_code) {
            return true;
        }
    }
    return false;
}
}