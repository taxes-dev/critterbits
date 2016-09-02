#include <critterbits.h>

namespace Critterbits {

Scene::~Scene() {
    if (this->tilemap != nullptr) {
        delete this->tilemap;
    }
}

void Scene::NotifyLoaded() {
    LOG_INFO("Scene::NotifyLoaded scene was loaded: " + this->scene_name);
    EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
        // if this scene has a tilemap, make sure it's ready to render
        if (!this->map_path.empty()) {
            LOG_INFO("Scene::NotifyLoaded(pre-update) beginning tile map preparation for scene " + this->scene_name);

            this->tilemap = new Tilemap(this->map_path);
            if (!this->tilemap->CreateTextures(this->map_scale)) {
                LOG_ERR("Scene::NotifyLoaded(pre-update) unable to generate textures for tilemap " + this->map_path);
            }
        }
        if (!this->sprites.LoadQueuedSprites()) {
            LOG_ERR("Scene::NotifyLoaded(pre-update) unable to load scene sprites for scene " + this->scene_name);
        }
        this->state = CBE_SCENE_ACTIVE;
    });
}

void Scene::NotifyUnloaded(bool unloading) {
    LOG_INFO("Scene::NotifyUnloaded scene was unloaded: " + this->scene_name);
    this->state = unloading ? CBE_SCENE_UNLOADED : CBE_SCENE_INACTIVE;
}
}