#pragma once
#ifndef CBANIM_HPP
#define CBANIM_HPP

#include <memory>
#include <vector>

#include "cbentity.hpp"

namespace Critterbits {

typedef enum { CBE_ANIM_STOPPED, CBE_ANIM_PAUSED, CBE_ANIM_PLAYING } AnimationState;

typedef struct KeyFrame {
    std::string property;
    std::string value;
    int duration;

    KeyFrame(const std::string & property, const std::string & value, int duration) :
        property(property), value(value), duration(duration) {};
} KeyFrame;

class Animation {
  public:
    std::string name;
    bool loop;

    Animation(const std::string & name) : name(name), loop(false) {};
    void AddKeyFrame(const KeyFrame & key_frame);
    void Animate(std::shared_ptr<Entity>, float);
    bool IsPlaying() { return this->state == CBE_ANIM_PLAYING; };
    void Pause() { this->state = CBE_ANIM_PAUSED; };
    void Play() { this->state = CBE_ANIM_PLAYING; };
    void Stop();

  private:
    AnimationState state{CBE_ANIM_STOPPED};
    int next_key_frame{0};
    int key_frame_count{0};
    float key_frame_delta{0.f};
    float next_key_frame_at{0.f};
    std::vector<KeyFrame> key_frames{};

    void AnimateKeyFrame(std::shared_ptr<Entity>, const KeyFrame &);
};
}
#endif