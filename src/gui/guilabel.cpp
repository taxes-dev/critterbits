#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>

namespace Critterbits {
namespace Gui {

void GuiLabel::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        if (this->bg_color.a > 0) {
            boxRGBA(renderer, clip_info.dest.x, clip_info.dest.y, clip_info.dest.right(), clip_info.dest.bottom(),
                    this->bg_color.r, this->bg_color.g, this->bg_color.b, this->bg_color.a);
        }
        std::string label_text{this->text};
        if (8 * label_text.length() + 4 > static_cast<size_t>(this->dim.w)) {
            label_text = label_text.substr(0, std::max((this->dim.w - 4) / 8, 0));
        }
        stringRGBA(renderer, clip_info.dest.x + 2, clip_info.dest.y + 2, label_text.c_str(), this->text_color.r,
                   this->text_color.g, this->text_color.b, this->text_color.a);
    }
}

void GuiLabel::OnResize() {
    // TODO: fonts!
    this->dim.w = 8 * this->text.length() + 4;
    this->dim.h = 10;
}

bool GuiLabel::OnStart() {
    if (!this->font_name.empty()) {
        LOG_INFO("GuiLabel::OnStart attempting to load font " + this->font_name);
        this->font_resource = FontManager::GetInstance().GetNamedFont(this->font_name);
    }
    return true;
}
}
}