#include <critterbits.h>

namespace Critterbits {

void EngineEventQueue::ExecuteCollision() {
    // take a copy of the collision queue in case the events modify it
    std::vector<CollisionEvent> events = this->collision;
    this->collision.clear();
    for (auto event : events) {
        event();
    }
}

void EngineEventQueue::ExecutePreUpdate() {
    // take a copy of the pre_update queue in case the events modify it
    std::vector<PreUpdateEvent> events = this->pre_update;
    this->pre_update.clear();
    for (auto event : events) {
        event();
    }
}

EngineEventQueue & EngineEventQueue::GetInstance() {
    static EngineEventQueue instance;
    return instance;
}

void EngineEventQueue::QueueCollision(const CollisionEvent event) { this->collision.push_back(event); }
void EngineEventQueue::QueuePreUpdate(const PreUpdateEvent event) { this->pre_update.push_back(event); }
}