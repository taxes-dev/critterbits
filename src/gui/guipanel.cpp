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
    // TODO: re-factor to use a single-dimension array
    std::vector<std::vector<CB_Point>> cell_sizes{static_cast<size_t>(this->grid_rows), std::vector<CB_Point>{static_cast<size_t>(this->grid_cols)}};
    // first discover the largest control size in each row and column
    for (auto & control : this->children) {
        int row = control->grid.at.y;
        int col = control->grid.at.x;
        int row_span = Clamp(control->grid.row_span, 1, this->grid_rows - row + 1);
        int col_span = Clamp(control->grid.col_span, 1, this->grid_cols - col + 1);
        control->Resize();
        // FIXME: hmm, this now causes things to flow and compact to the left, which is not exactly what I wanted
        for (int i = 0; i < row_span; i++) {
            for (int j = 0; j < col_span; j++) {
                cell_sizes.at(row + i).at(col + j) = {
                    std::max(cell_sizes.at(row + i).at(col + j).x, control->dim.w / col_span),
                    std::max(cell_sizes.at(row + i).at(col + j).y, control->dim.h / row_span)
                    };
            }
        }
    }
    // now set dimensions on each control based on the largest dimensions needed for each row/column
    for (auto & control : this->children) {
        int row = control->grid.at.y;
        int col = control->grid.at.x;
        control->dim.x = 0;
        std::for_each(cell_sizes.at(row).begin(), cell_sizes.at(row).begin() + col,
                    [this, &control](const CB_Point & xy) { control->dim.x += xy.x + this->grid_padding.x; });
        control->dim.y = 0;
        std::for_each(cell_sizes.begin(), cell_sizes.begin() + row,
                    [this, &control, &col](const std::vector<CB_Point> & cols) { control->dim.y += cols.at(col).y + this->grid_padding.y; });
    }
}
}
}