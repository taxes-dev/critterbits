#include <algorithm>

#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {
namespace Gui {
GuiPanel::GuiPanel() : Entity() { this->debug = Engine::GetInstance().config->debug.draw_gui_rects; }

CB_ViewClippingInfo GuiPanel::AdjustClipToClientArea(const CB_ViewClippingInfo & clip_info,
                                                     const CB_Rect & control_dim) const {
    CB_ViewClippingInfo client_area;
    client_area.source = control_dim;
    client_area.dest.x = clip_info.dest.x + this->decoration.GetBorderLeftScaled() + control_dim.x;
    client_area.dest.y = clip_info.dest.y + this->decoration.GetBorderTopScaled() + control_dim.y;
    client_area.dest.w = control_dim.w;
    client_area.dest.h = control_dim.h;
    client_area.z_index = clip_info.z_index;

    if (client_area.dest.right() > clip_info.dest.right() - this->decoration.GetBorderRightScaled()) {
        client_area.dest.w = clip_info.dest.right() - client_area.dest.x - this->decoration.GetBorderRightScaled();
    }
    if (client_area.dest.bottom() > clip_info.dest.bottom() - this->decoration.GetBorderBottomScaled()) {
        client_area.dest.h = clip_info.dest.bottom() - client_area.dest.y - this->decoration.GetBorderBottomScaled();
    }

    return client_area;
}

void GuiPanel::Close() {
    this->state = EntityState::Inactive;
    if (this->destroy_on_close) {
        this->MarkDestroy();
    }
}

void GuiPanel::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        if (this->decoration.texture != nullptr) {
            SDLx::SDL_RenderTextureClipped(renderer, this->decoration.SliceTo(this->dim.w, this->dim.h),
                                           clip_info.source, clip_info.dest);
        }
        for (auto & control : this->children) {
            if (control->IsActive()) {
                CB_ViewClippingInfo control_clip = this->AdjustClipToClientArea(clip_info, control->dim);
                control->Render(renderer, control_clip);
            }
        }
    }
}

void GuiPanel::OnDebugRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        rectangleRGBA(renderer, clip_info.dest.x, clip_info.dest.y, clip_info.dest.right(), clip_info.dest.bottom(),
                      255, 69, 0, 127);
        std::string label = this->panel_name + ":" + std::to_string(this->entity_id);
        boxRGBA(renderer, clip_info.dest.x, clip_info.dest.bottom(), clip_info.dest.x + label.length() * 8 + 2,
                clip_info.dest.bottom() + 10, 255, 69, 0, 127);
        stringRGBA(renderer, clip_info.dest.x + 1, clip_info.dest.bottom() + 1, label.c_str(), 255, 255, 255, 255);
    }
}

void GuiPanel::Open() {
    if (!this->IsDestroyed()) {
        this->state = EntityState::Active;
    }
}

void GuiPanel::Reflow(const CB_Rect & parent_rect) {
    this->dim = this->flex.FlexBasedOn(parent_rect);
    CB_Rect client_rect{0, 0,
                        this->dim.w - this->decoration.GetBorderLeftScaled() - this->decoration.GetBorderRightScaled(),
                        this->dim.h - this->decoration.GetBorderTopScaled() - this->decoration.GetBorderBottomScaled()};
    this->layout->Reflow(client_rect);
    for (auto & control : this->children) {
        control->dim = this->layout->GetCellRect(control->grid.at.x, control->grid.at.y, control->grid.row_span,
                                                 control->grid.col_span);
    }
}
}
}