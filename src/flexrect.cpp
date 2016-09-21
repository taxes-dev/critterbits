#include <cb/critterbits.hpp>

namespace Critterbits {
CB_Rect FlexRect::FlexBasedOn(const CB_Rect & parent) const {
    CB_Rect new_rect;

    // determine vertical positioning
    if (this->top != FlexRect::FLEX && (this->height != FlexRect::FLEX || this->bottom != FlexRect::FLEX)) {
        // anchor to top
        new_rect.y = parent.y + this->top;
        if (this->height != FlexRect::FLEX) {
            new_rect.h = this->height;
        } else {
            new_rect.h = parent.bottom() - this->bottom - new_rect.y;
        }
    } else if (this->height != FlexRect::FLEX && this->bottom != FlexRect::FLEX) {
        // anchor to bottom
        new_rect.y = parent.bottom() - this->bottom - this->height;
        new_rect.h = this->height;
    } else if (this->height != FlexRect::FLEX && this->top == FlexRect::FLEX && this->bottom == FlexRect::FLEX) {
        // center vertically
        new_rect.y = parent.y + parent.h / 2 - this->height / 2;
        new_rect.h = this->height; 
    } else {
        LOG_ERR("FlexRect::FlexBasedOn insufficient information to determine vertical positioning");
        new_rect.y = parent.y;
        new_rect.h = 0;
    }

    // determine horizontal positioning
    if (this->left != FlexRect::FLEX && (this->width != FlexRect::FLEX || this->right != FlexRect::FLEX)) {
        // anchor to left
        new_rect.x = parent.x + this->left;
        if (this->width != FlexRect::FLEX) {
            new_rect.w = this->width;
        } else {
            new_rect.w = parent.right() - this->right - new_rect.x;
        }
    } else if (this->width != FlexRect::FLEX && this->right != FlexRect::FLEX) {
        // anchor to right
        new_rect.x = parent.right() - this->right - this->width;
        new_rect.w = this->width;
    } else if (this->width != FlexRect::FLEX && this->left == FlexRect::FLEX && this->right == FlexRect::FLEX) {
        // center horizontally
        new_rect.x = parent.x + parent.w / 2 - this->width / 2;
        new_rect.w = this->width;
    } else {
        LOG_ERR("FlexRect::FlexBasedOn insufficient information to determine horizontal positioning");
        new_rect.x = parent.x;
        new_rect.w = 0;
    }

    return new_rect;
}
}