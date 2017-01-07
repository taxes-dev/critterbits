#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Animation {
void TransformAnimation::Animate(std::shared_ptr<Entity> entity, float delta_time) {
    if (this->IsPlaying()) {
        this->elapsed += delta_time;
        if (this->elapsed < this->duration) {
            float percent = Clampf(this->elapsed / this->duration, 0.f, 1.f);
            this->AnimateValues(entity, percent);
        } else {
            this->Stop();
            if (this->callback != nullptr) {
                if (auto ent = this->callback->owner.lock()) {
                    ent->script->QueueCallback(std::move(this->callback));
                }
            }
        }
    }
}

float TransformAnimation::GetTransformedValue(float start, float end, float percent) {
    switch (this->algorithm) {
        case TransformAlgorithm::Lerp:
            return Lerp(start, end, percent);
        case TransformAlgorithm::QuadEaseIn:
            return QuadEaseIn(start, end, percent);
    }
    return 0.f;
}

CB_Point TransformAnimation::GetTransformedValue(CB_Point start, CB_Point end, float percent) {
    switch (this->algorithm) {
        case TransformAlgorithm::Lerp:
            return Lerp2D(start, end, percent);
        case TransformAlgorithm::QuadEaseIn:
            return QuadEaseIn2D(start, end, percent);
    }
    return {0, 0};
}

void TransformAnimation::SetCallback(std::unique_ptr<Scripting::CB_ScriptCallback> callback) {
    this->callback = std::move(callback);
}

}
}