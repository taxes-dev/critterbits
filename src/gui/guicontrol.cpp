#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {
namespace Gui {
GuiControl::GuiControl() : Entity() { this->debug = Engine::GetInstance().config->debug.draw_gui_rects; }

void GuiControl::OnDebugRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        rectangleRGBA(renderer, clip_info.dest.x, clip_info.dest.y, clip_info.dest.right(), clip_info.dest.bottom(),
                      255, 69, 0, 127);
        std::string label = this->dim.to_string();
        boxRGBA(renderer, clip_info.dest.x, clip_info.dest.bottom(), clip_info.dest.x + label.length() * 8 + 2,
                clip_info.dest.bottom() + 10, 255, 69, 0, 127);
        stringRGBA(renderer, clip_info.dest.x + 1, clip_info.dest.bottom() + 1, label.c_str(), 255, 255, 255, 255);
        std::string grid = this->grid.to_string();
        boxRGBA(renderer, clip_info.dest.right() - 8 * grid.length() - 2, clip_info.dest.y - 10,
                clip_info.dest.right() - 1, clip_info.dest.y - 1, 255, 69, 0, 127);
        stringRGBA(renderer, clip_info.dest.right() - 8 * grid.length() - 1, clip_info.dest.y - 9, grid.c_str(),
                    255, 255, 255, 255);
    }
}

void GuiControl::Resize() {
    if (this->resize_behavior == ResizeBehavior::Resize) {
        this->OnResize();
        this->dim.w = Clamp(this->dim.w, this->min_w, this->max_w);
        this->dim.h = Clamp(this->dim.h, this->min_h, this->max_h);
    }
}
}
}