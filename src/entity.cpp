#include <critterbits.hpp>

namespace Critterbits {
void Entity::Start() {
    if (!this->started) {
        if (this->OnStart()) {
            if (this->HasScript()) {
                this->script->CallStart(shared_from_this());
            }
            this->started = true;
        }
    }
}

void Entity::Update(float delta_time) {
    if (this->HasScript()) {
        this->script->CallUpdate(shared_from_this(), delta_time * this->time_scale);
    }
    this->OnUpdate(delta_time * this->time_scale);
}
}