#include "cbcoord.h"
#include "cbviewport.h"

namespace Critterbits {

CB_Rect Viewport::GetViewableRect(CB_Rect & entity_dim) const {
    CB_Rect view_rect;
    // set x & y relative to viewport
    view_rect.x = entity_dim.x - this->dim.x;
    view_rect.y = entity_dim.y - this->dim.y;
    view_rect.w = 0; // entity_dim.w;
    view_rect.h = 0; // entity_dim.h;
    // TODO: set clip
    if (view_rect.x < 0) {
        view_rect.w = entity_dim.w - view_rect.x * -1;
        view_rect.x = 0;
    }
    if (view_rect.y < 0) {
        view_rect.h = entity_dim.h - view_rect.y * -1;
        view_rect.y = 0;
    }
    return view_rect;
}
}