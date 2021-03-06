#include <cb/critterbits.hpp>

namespace Critterbits {

CB_ViewClippingInfo Viewport::GetStaticViewableRect(CB_Rect & entity_dim, const ZIndex & z_index) const {
    // assume x & y are relative to viewport instead of world
    return this->GetViewableRect(entity_dim, z_index, entity_dim.x, entity_dim.y);
}

CB_ViewClippingInfo Viewport::GetViewableRect(CB_Rect & entity_dim, const ZIndex & z_index) const {
    // set x & y relative to viewport
    return this->GetViewableRect(entity_dim, z_index, entity_dim.x - this->dim.x, entity_dim.y - this->dim.y);
}

CB_ViewClippingInfo Viewport::GetViewableRect(CB_Rect & entity_dim, const ZIndex & z_index, int dest_x, int dest_y) const {
    CB_ViewClippingInfo view_clip;
    view_clip.z_index = z_index;
    view_clip.dest.x = dest_x;
    view_clip.dest.y = dest_y;

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

    // set destination size to same as source
    view_clip.dest.w = view_clip.source.w;
    view_clip.dest.h = view_clip.source.h;

    // determine if clipping is necessary (right, bottom)
    if (view_clip.dest.right() > this->dim.w) {
        view_clip.source.w -= view_clip.dest.right() - this->dim.w;
        view_clip.dest.w = view_clip.source.w;
    }
    if (view_clip.dest.bottom() > this->dim.h) {
        view_clip.source.h -= view_clip.dest.bottom() - this->dim.h;
        view_clip.dest.h = view_clip.source.h;
    }

    return view_clip;
}

void Viewport::SetEntityToFollow(std::shared_ptr<Entity> entity) {
    if (entity != nullptr) {
        this->entity_to_follow = entity;
        this->entity_center_view = CB_Point::CenterInside(this->dim.w, this->dim.h, entity->dim.w, entity->dim.h);
    } else {
        this->entity_to_follow = std::shared_ptr<Entity>{nullptr};
    }
}

void Viewport::OnUpdate(float delta_time) {
    if (auto entity = this->entity_to_follow.lock()) {
        // Re-center viewport on followed entity
        this->dim.x = entity->dim.x - this->entity_center_view.x;
        this->dim.y = entity->dim.y - this->entity_center_view.y;
    }
}
}