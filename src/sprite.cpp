#include <critterbits.h>
#include <SDL_image.h>

namespace Critterbits {
    Sprite::~Sprite() {
if (this->sprite_sheet != nullptr) {
    SDLx::SDL_CleanUp(this->sprite_sheet);
}
    }

    void Sprite::NotifyLoaded() {
        LOG_INFO("Sprite::NotifyLoaded sprite was loaded " + this->sprite_sheet_path);
    }

    void Sprite::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {

    }

}