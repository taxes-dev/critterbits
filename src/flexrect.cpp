#include <cb/critterbits.hpp>

namespace Critterbits {
CB_Rect FlexRect::FlexBasedOn(const CB_Rect & parent) const {
    CB_Rect new_rect;
    // FIXME: this is kind of mess, probably a better way to do it
    switch (this->anchor) {
        case AnchorPoint::TopLeft:
            new_rect.x = parent.x + this->left;
            new_rect.y = parent.y + this->top;
            if (this->right == FlexRect::FLEX) {
                new_rect.w = this->width;
            } else {
                new_rect.w = parent.right() - this->right - new_rect.x; 
            }
            if (this->bottom == FlexRect::FLEX) {
                new_rect.h = this->height;
            } else {
                new_rect.h = parent.bottom() - this->bottom - new_rect.y;
            }
            break;
        case AnchorPoint::TopRight:
            new_rect.y = parent.y + this->top;
            if (this->left == FlexRect::FLEX) {
                new_rect.w = this->width;
                new_rect.x = parent.right() - this->right - new_rect.w;
            } else {
                new_rect.x = this->left;
                new_rect.w = parent.right() - this->right - this->left;
            }
            if (this->bottom == FlexRect::FLEX) {
                new_rect.h = this->height;
            } else {
                new_rect.h = parent.bottom() - this->bottom - new_rect.y;
            }
            break;
        case AnchorPoint::Center:
            new_rect.w = this->width;
            new_rect.h = this->height;
            new_rect.xy(CB_Point::CenterInside(parent.w, parent.h, new_rect.w, new_rect.h));
            break;
        case AnchorPoint::BottomLeft:
            new_rect.x = parent.x + this->left;
            if (this->top == FlexRect::FLEX) {
                new_rect.y = parent.bottom() - this->bottom - this->height;
                new_rect.h = this->height;
            } else {
                new_rect.y = parent.y + this->top;
                new_rect.h = parent.bottom() - this->bottom - this->top;
            }
            if (this->right == FlexRect::FLEX) {
                new_rect.w = this->width;
            } else {
                new_rect.w = parent.right() - this->right - new_rect.x;
            }
            break;
        case AnchorPoint::BottomRight:
            if (this->top == FlexRect::FLEX) {
                new_rect.y = parent.bottom() - this->bottom - this->height;
                new_rect.h = this->height;
            } else {
                new_rect.y = parent.y + this->top;
                new_rect.h = parent.bottom() - this->bottom - this->top;
            }
            if (this->left == FlexRect::FLEX) {
                new_rect.w = this->width;
                new_rect.x = parent.right() - this->right - new_rect.w;
            } else {
                new_rect.x = this->left;
                new_rect.w = parent.right() - this->right - this->left;
            }
            break;
    }
    return new_rect;
}
}