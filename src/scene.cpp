#include <cb/critterbits.hpp>

namespace Critterbits {

Scene::~Scene() {}

void Scene::NotifyLoaded() {
    LOG_INFO("Scene::NotifyLoaded scene was loaded: " + this->scene_name);
    EngineEventQueue::GetInstance().QueuePreUpdate((PreUpdateEvent)[this]() {
        // if this scene has a tilemap, make sure it's ready to render
        if (!this->map_path.empty()) {
            LOG_INFO("Scene::NotifyLoaded(pre-update) beginning tile map preparation for scene " + this->scene_name);

            this->tilemap = std::make_shared<Tilemap>(this->map_path);
            if (!this->tilemap->CreateTextures(this->map_scale)) {
                LOG_ERR("Scene::NotifyLoaded(pre-update) unable to generate textures for tilemap " + this->map_path);
            }
        }

        // if this scene has a scene-wide script, load it and attach to a special sprite
        if (!this->script_path.empty()) {
            LOG_INFO("Scene::NotifyLoaded(pre-update) attempting to load script " + this->script_path);
            std::shared_ptr<Scripting::Script> script = Engine::GetInstance().scripts.LoadScript(this->script_path);
            if (script != nullptr) {
                std::shared_ptr<Sprite> scene_sprite{std::make_shared<Sprite>()};
                scene_sprite->sprite_name = ":" + this->scene_name;
                scene_sprite->script = std::move(script);
                scene_sprite->state = EntityState::Active;
                this->sprites.sprites.push_back(std::move(scene_sprite));
            }
        }

        this->state = SceneState::Active;
    });
}

void Scene::NotifyUnloaded(bool unloading) {
    LOG_INFO("Scene::NotifyUnloaded scene was unloaded: " + this->scene_name);
    this->state = unloading ? SceneState::Unloaded : SceneState::Inactive;
}
}