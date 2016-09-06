#include <algorithm>

#include <SDL.h>
#include <critterbits.h>

namespace Critterbits {
    float EngineCounters::GetDeltaFromRemainingFrameTime() {
        return std::min(this->delta_time, this->frame_time / 1000.0f);
    }

    void EngineCounters::NewFrame() {
        this->ticks = SDL_GetTicks();
        this->frame_time = this->ticks - this->last_ticks;
        this->last_ticks = this->ticks;
        this->frame_count++;
        this->render_count = 0;
        if (this->frame_count % 10 == 0) {
            this->fps = (this->fps + 1000.0f / this->frame_time) / 2.0f;
        }
    }

    void EngineCounters::RenderedEntity() {
        this->render_count++;
    }

    void EngineCounters::Reset() {
        this->fps = 0.;
        this->ticks = 0;
        this->last_ticks = 0;
        this->frame_time = 0;
        this->frame_count = 0;
        this->render_count = 0;
        this->update_count = 0;
    }

    void EngineCounters::Updated() {
        this->update_count++;
        this->frame_time -= this->delta_time * 1000;
    }
}