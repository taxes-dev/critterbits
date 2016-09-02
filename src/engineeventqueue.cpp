#include <critterbits.h>

namespace Critterbits {

void EngineEventQueue::ExecutePreUpdate() {
    for (auto & event : this->pre_update) {
        event();
    }
    this->pre_update.clear();
}

EngineEventQueue & EngineEventQueue::GetInstance() {
    static EngineEventQueue instance;
    return instance;
}

void EngineEventQueue::QueuePreUpdate(const PreUpdateEvent & event) { this->pre_update.push_back(event); }
}