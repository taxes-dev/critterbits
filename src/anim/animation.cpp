#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Animation {
void Animation::Pause() {
    this->state = AnimationState::Paused;
    this->OnPause();
}

void Animation::Play() {
    this->state = AnimationState::Playing;
    this->OnPlay();
}

void Animation::Stop() {
    this->state = AnimationState::Stopped;
    this->OnStop();
}
}
}