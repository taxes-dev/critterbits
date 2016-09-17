#include <critterbits.hpp>

namespace Critterbits {
void Animation::AddKeyFrame(const KeyFrame & key_frame) {
    this->key_frames.push_back(key_frame);
    this->key_frame_count++;
}

void Animation::Animate(std::shared_ptr<Entity> entity, float delta_time) {
    if (this->IsPlaying()) {
        this->key_frame_delta += delta_time;
        if (this->key_frame_delta >= this->next_key_frame_at) {
            if ((size_t)this->next_key_frame < this->key_frames.size()) {
                const KeyFrame & key_frame = this->key_frames.at(this->next_key_frame);
                this->AnimateKeyFrame(entity, key_frame);
                this->key_frame_delta = 0.f;
                if (++this->next_key_frame == this->key_frame_count) {
                    this->next_key_frame = 0;
                    if (!this->loop) {
                        this->Stop();
                    }
                } 
                this->next_key_frame_at = (float)key_frame.duration / 1000.f;
            } else {
                LOG_ERR("Animation::Animate animated past end of key frames");
                this->Stop();
            }
        }
    }
}

void Animation::AnimateKeyFrame(std::shared_ptr<Entity> entity, const KeyFrame & key_frame) {
    // entity properties

    // sprite properties
    if (entity->GetEntityType() == EntityType::Sprite) {
        std::shared_ptr<Sprite> sprite = std::dynamic_pointer_cast<Sprite>(entity);
        if (key_frame.property == "flip_x") {
            sprite->flip_x = key_frame.value == "true";
        } else if (key_frame.property == "flip_y") {
            sprite->flip_y = key_frame.value == "true";
        } else if (key_frame.property == "frame.current") {
            int new_frame = std::stol(key_frame.value);
            sprite->SetFrame(new_frame);
        }
    }
}

void Animation::Stop() {
    this->state = AnimationState::Stopped;
    this->next_key_frame = 0;
    this->next_key_frame_at = 0.f;
    this->key_frame_delta = 0.f;
}
}