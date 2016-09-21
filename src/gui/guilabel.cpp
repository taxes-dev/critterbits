#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {
namespace Gui {

void GuiLabel::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        if (this->bg_color.a > 0) {
            boxRGBA(renderer, clip_info.dest.x, clip_info.dest.y, clip_info.dest.right(), clip_info.dest.bottom(), this->bg_color.r, this->bg_color.g, this->bg_color.b, this->bg_color.a);
        }
        stringRGBA(renderer, clip_info.dest.x + 2, clip_info.dest.y + 2, this->text.c_str(), this->text_color.r, this->text_color.g, this->text_color.b, this->text_color.a);
    }
}

void GuiLabel::OnResize() {
    // TODO: fonts!
    this->dim.w = 8 * this->text.length() + 4;
    this->dim.h = 10;
}
}
}