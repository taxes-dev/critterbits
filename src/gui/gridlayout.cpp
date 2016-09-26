#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Gui {
namespace {
/*
 * Begin support functions
 */
void ArrangeCells(int parent_dim, int parent_offset, int padding, std::vector<CB_GridDescriptor> * descriptors) {
    int remaining_dim = parent_dim;
    int pos = parent_offset;
    std::vector<size_t> flexors;
    for (size_t i = 0; i < descriptors->size(); i++) {
        CB_GridDescriptor & desc = descriptors->at(i);
        desc.position = pos;
        if (desc.flex) {
            flexors.push_back(i);
        } else {
            desc.actual_size = std::min(desc.desired_size, remaining_dim - padding);
            remaining_dim -= desc.actual_size + padding;
            pos += desc.actual_size + padding;
        }
    }
    remaining_dim -= padding * flexors.size();
    if (flexors.size() > 0 && remaining_dim > 0) {
        int per_cell_dim = remaining_dim / flexors.size();
        for (auto & flex_at : flexors) {
            descriptors->at(flex_at).actual_size = per_cell_dim;
            for (size_t i = flex_at + 1; i < descriptors->size(); i++) {
                descriptors->at(i).position += per_cell_dim + padding;
            }
        }
    }
}
/*
 * End support functions
 */
}

CB_Rect GridLayout::GetCellRect(int cell_x, int cell_y, int row_span, int col_span) const {
    CB_Rect cell_rect;
    int r = Clamp(row_span, 1, this->GetRowCount() - cell_y + 1);
    int c = Clamp(col_span, 1, this->GetColumnCount() - cell_x + 1);

    for (int i = cell_y; i < cell_y + r; i++) {
        const CB_GridDescriptor & row = this->rows.at(i);
        if (i == cell_y) {
            cell_rect.y = row.position;
        }
        cell_rect.h += row.actual_size;
    }
    cell_rect.h += (r - 1) * this->v_padding;

    for (int j = cell_x; j < cell_x + c; j++) {
        const CB_GridDescriptor & col = this->cols.at(j);
        if (j == cell_x) {
            cell_rect.x = col.position;
        }
        cell_rect.w += col.actual_size;
    }
    cell_rect.w += (c - 1) * this->h_padding;

    return cell_rect;
}

void GridLayout::Reflow(const CB_Rect & parent, bool use_parent_offset) {
    ArrangeCells(parent.w, use_parent_offset ? parent.x : 0, this->h_padding, &this->cols);
    ArrangeCells(parent.h, use_parent_offset ? parent.y : 0, this->v_padding, &this->rows);
}

void GridLayout::SetColumnWidth(int column_index, int width) {
    CB_GridDescriptor & col = this->cols.at(column_index);
    if (width == GridLayout::FLEX) {
        col.flex = true;
        col.desired_size = 0;
    } else {
        col.flex = false;
        col.desired_size = width;
    }
}

void GridLayout::SetRowHeight(int row_index, int height) {
    CB_GridDescriptor & row = this->rows.at(row_index);
    if (height == GridLayout::FLEX) {
        row.flex = true;
        row.desired_size = 0;
    } else {
        row.flex = false;
        row.desired_size = height;
    }
}
}
}