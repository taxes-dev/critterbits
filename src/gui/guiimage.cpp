#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Gui {

CB_Rect GuiImage::GetImageSize() {
    if (this->image_size.w == 0 && this->image_size.h == 0 && this->image_texture != nullptr) {
        SDL_QueryTexture(this->image_texture.get(), NULL, NULL, &this->image_size.w, &this->image_size.h);
    }
    return this->image_size;
}

void GuiImage::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        if (this->image_texture != nullptr) {
            CB_Rect dest = clip_info.dest;
            CB_Rect src = this->GetImageSize();
            if (this->image_mode == GuiImageMode::Actual) {
                dest.w = std::min(dest.w, src.w);
                dest.h = std::min(dest.h, src.h);
                src.w = dest.w;
                src.h = dest.h;
            }
            SDLx::SDL_RenderTextureClipped(renderer, this->image_texture.get(), src, dest);
        }
    }
}

void GuiImage::OnResize() {
    if (this->image_texture != nullptr) {
        CB_Rect size = this->GetImageSize();
        this->dim.w = size.w;
        this->dim.h = size.h;
    } else {
        this->dim.w = 0;
        this->dim.h = 0;
    }
}
}
}