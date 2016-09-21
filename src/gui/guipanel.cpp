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
    client_area.dest.x = clip_info.dest.x + this->decoration.border.left * this->decoration.scale + control_dim.x;
    client_area.dest.y = clip_info.dest.y + this->decoration.border.top * this->decoration.scale + control_dim.y;
    client_area.dest.w = control_dim.w;
    client_area.dest.h = control_dim.h;
    client_area.z_index = clip_info.z_index;

    if (client_area.dest.right() > clip_info.dest.right() - this->decoration.border.right * this->decoration.scale) {
        client_area.dest.w =
            clip_info.dest.right() - client_area.dest.x - this->decoration.border.right * this->decoration.scale;
    }
    if (client_area.dest.bottom() > clip_info.dest.bottom() - this->decoration.border.bottom * this->decoration.scale) {
        client_area.dest.h =
            clip_info.dest.bottom() - client_area.dest.y - this->decoration.border.bottom * this->decoration.scale;
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
    std::vector<int> row_widths;
    row_widths.resize(this->grid_rows);
    std::vector<int> col_heights;
    col_heights.resize(this->grid_cols);
    // first discover the largest control size in each row and column
    for (auto & control : this->children) {
        int row = control->grid.y;
        int col = control->grid.x;
        control->Resize();
        row_widths.at(row) = std::max(row_widths.at(row), control->dim.w);
        col_heights.at(col) = std::max(col_heights.at(col), control->dim.h);
    }
    // now set dimensions on each control based on the largest dimensions needed for each row/column
    for (auto & control : this->children) {
        int row = control->grid.y;
        int col = control->grid.x;
        control->dim.x = 0;
        std::for_each(row_widths.begin(), row_widths.begin() + row,
                      [this, &control](int w) { control->dim.x += w + this->grid_padding.x; });
        control->dim.y = 0;
        std::for_each(col_heights.begin(), col_heights.begin() + col,
                      [this, &control](int h) { control->dim.y += h + this->grid_padding.y; });
        control->dim.w = row_widths.at(row);
        control->dim.h = col_heights.at(col);
    }
}
}
}