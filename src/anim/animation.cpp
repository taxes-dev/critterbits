#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Animation {
void Animation::Destroy() {
    if (this->state != AnimationState::Destroyed) {
        if (this->IsPlaying()) {
            this->Stop();
        }
        this->state = AnimationState::Destroyed;
        this->OnDestroy();
    }
}

void Animation::Pause() {
    if (this->state != AnimationState::Destroyed && this->state != AnimationState::Paused) {
        this->state = AnimationState::Paused;
        this->OnPause();
    }
}

void Animation::Play() {
    if (this->state != AnimationState::Destroyed && this->state != AnimationState::Playing) {
        this->state = AnimationState::Playing;
        this->OnPlay();
    }
}

void Animation::Stop() {
    if (this->state != AnimationState::Destroyed && this->state != AnimationState::Stopped) {
        this->state = AnimationState::Stopped;
        this->OnStop();
    }
}
}
}