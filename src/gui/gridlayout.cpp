#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Gui {
CB_Rect GridLayout::GetCellRect(int cell_x, int cell_y, int row_span, int col_span) const {
    CB_Rect cell_rect;
    int r = Clamp(row_span, 1, this->GetRowCount() - cell_y + 1);
    int c = Clamp(col_span, 1, this->GetColumnCount() - cell_x + 1);
    for (int i = cell_y; i < cell_y + r; i++) {
        const CB_GridRow & row = this->rows.at(i);
        if (i == cell_y) {
            cell_rect.y = row.dim.y;
        }
        cell_rect.h += row.dim.h;
    }
    cell_rect.h += (r - 1) * this->v_padding;

    for (int j = cell_x; j < cell_x + c; j++) {
        const CB_GridColumn & col = this->cols.at(j);
        if (j == cell_x) {
            cell_rect.x = col.dim.x;
        }
        cell_rect.w += col.dim.w;
    }
    cell_rect.w += (c - 1) * this->h_padding;

    return cell_rect;
}

void GridLayout::Reflow(const CB_Rect & parent, bool use_parent_offset) {
    // calculate column widths
    int remaining_width = parent.w;
    int x = use_parent_offset ? parent.x : 0;
    int flex_column = -1;
    for (size_t i = 0; i < this->cols.size(); i++) {
        CB_GridColumn & col = this->cols.at(i); 
        col.dim.x = x;
        if (col.flex_width) {
            if (flex_column != -1) {
                LOG_ERR("GridLayout::Reflow more than one flex column specified");
            }
            flex_column = i;
        } else {
            col.dim.w = std::min(col.width, remaining_width - this->h_padding); 
            remaining_width -= col.dim.w + this->h_padding;
            x += col.dim.w + this->h_padding;
        } 
    }
    if (flex_column > -1 && (remaining_width - this->h_padding) > 0) {
        this->cols.at(flex_column).dim.w = remaining_width - this->h_padding;
        for (size_t i = flex_column + 1; i < this->cols.size(); i++) {
            this->cols.at(i).dim.x += remaining_width - this->h_padding;
        }
    }

    // calculate row heights
    int remaining_height = parent.h;
    int y = use_parent_offset ? parent.y : 0;
    int flex_row = -1;
    for (size_t i = 0; i < this->rows.size(); i++) {
        CB_GridRow & row = this->rows.at(i); 
        row.dim.y = y;
        if (row.flex_height) {
            if (flex_row != -1) {
                LOG_ERR("GridLayout::Reflow more than one flex row specified");
            }
            flex_row = i;
        } else {
            row.dim.h = std::min(row.height, remaining_height - this->v_padding); 
            remaining_height -= row.dim.h + this->v_padding;
            y += row.dim.h + this->v_padding;
        } 
    }
    if (flex_row > -1 && (remaining_height - this->v_padding) > 0) {
        this->rows.at(flex_row).dim.h = remaining_height - this->v_padding;
        for (size_t i = flex_row + 1; i < this->rows.size(); i++) {
            this->rows.at(i).dim.y += remaining_height - this->v_padding;
        }
    }
}

void GridLayout::SetColumnWidth(int column_index, int width) {
    CB_GridColumn & col = this->cols.at(column_index);
    if (width == GridLayout::FLEX) {
        col.flex_width = true;
    } else {
        col.flex_width = false;
        col.width = width;
    }
}

void GridLayout::SetRowHeight(int row_index, int height) {
    CB_GridRow & row = this->rows.at(row_index);
    if (height == GridLayout::FLEX) {
        row.flex_height = true;
    } else {
        row.flex_height = false;
        row.height = height;
    }
}
}
}