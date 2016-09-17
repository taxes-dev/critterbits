#pragma once
#ifndef CBSCENE_HPP
#define CBSCENE_HPP

#include <memory>
#include <string>
#include <vector>

#include "cbsprite.hpp"
#include "cbtilemap.hpp"

#define CB_SCENE_PATH "scenes"
#define CB_FIRST_SCENE "startup"
#define CB_SCENE_EXT ".toml"

namespace Critterbits {

enum class SceneState { New, Active, Inactive, Unloaded };

class Scene {
  public:
    bool persistent{false};
    std::string scene_name;
    std::string map_path;
    float map_scale{1.0f};
    SceneState state{SceneState::New};
    SpriteManager sprites;

    Scene(){};
    ~Scene();
    std::shared_ptr<Tilemap> GetTilemap() { return this->tilemap; };
    bool HasTilemap() { return this->tilemap != nullptr; };
    void NotifyLoaded();
    void NotifyUnloaded(bool);

  private:
    std::shared_ptr<Tilemap> tilemap;
};

class SceneManager {
  public:
    std::shared_ptr<Scene> current_scene;

    SceneManager(){};
    ~SceneManager();
    bool IsCurrentSceneActive() {
        return this->current_scene != nullptr && this->current_scene->state == SceneState::Active;
    };
    bool LoadScene(const std::string &);
    std::string GetScenePath(const std::string &);

  private:
    std::string scene_path;
    std::vector<std::shared_ptr<Scene>> loaded_scenes;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    void UnloadCurrentScene();
};
}
#endif