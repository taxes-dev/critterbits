#include <critterbits.h>

namespace Critterbits {

CB_ViewClippingInfo Viewport::GetViewableRect(CB_Rect & entity_dim) const {
    CB_ViewClippingInfo view_clip;
    // set x & y relative to viewport
    view_clip.dest.x = entity_dim.x - this->dim.x;
    view_clip.dest.y = entity_dim.y - this->dim.y;

    // determine if clipping is necessary (left, top)
    if (view_clip.dest.x < 0) {
        view_clip.source.x = view_clip.dest.x * -1;
        view_clip.source.w = entity_dim.w - view_clip.source.x;
        view_clip.dest.x = 0;
    } else {
        view_clip.source.x = 0;
        view_clip.source.w = entity_dim.w;
    }
    if (view_clip.dest.y < 0) {
        view_clip.source.y = view_clip.dest.y * -1;
        view_clip.source.h = entity_dim.h - view_clip.source.y;
        view_clip.dest.y = 0;
    } else {
        view_clip.source.y = 0;
        view_clip.source.h = entity_dim.h;
    }

    // determine if clipping is necessary (right, bottom)
    if (view_clip.dest.right() > this->dim.w) {
        view_clip.source.w -= view_clip.dest.right() - this->dim.w;
    }
    if (view_clip.dest.bottom() > this->dim.h) {
        view_clip.source.h -= view_clip.dest.bottom() - this->dim.h;
    }

    // set destination size to same as source
    view_clip.dest.w = view_clip.source.w;
    view_clip.dest.h = view_clip.source.h;

    return view_clip;
}
}