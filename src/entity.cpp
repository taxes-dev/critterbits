#include <cb/critterbits.hpp>

namespace Critterbits {
entity_id_t next_entity_id = CB_ENTITY_ID_FIRST;

void Entity::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    if (this->IsActive()) {
        this->OnRender(renderer, clip_rect);
        if (this->debug) {
            this->OnDebugRender(renderer, clip_rect);
        }
    }
}

void Entity::Start() {
    if (!this->started) {
        if (this->OnStart()) {
            this->state = EntityState::Active;
            if (this->HasScript()) {
                this->script->CallStart(shared_from_this());
            }
            this->started = true;
        }
    }
}

void Entity::Update(float delta_time) {
    if (this->IsActive()) {
        float scaled_delta_time = delta_time * this->time_scale;
        if (this->HasScript()) {
            this->script->CallUpdate(shared_from_this(), scaled_delta_time);
        }
        this->OnUpdate(scaled_delta_time);
    }
}
}