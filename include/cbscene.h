#pragma once
#ifndef CBSCENE_H
#define CBSCENE_H

#include <list>
#include <memory>
#include <string>

#include "cbsprite.h"
#include "cbtilemap.h"

#define CB_SCENE_PATH "scenes"
#define CB_FIRST_SCENE "startup"
#define CB_SCENE_EXT ".yml"

namespace Critterbits {

typedef enum { CBE_SCENE_NEW, CBE_SCENE_ACTIVE, CBE_SCENE_INACTIVE, CBE_SCENE_UNLOADED } SceneState;

class Scene {
  public:
    bool persistent = false;
    std::string scene_name;
    std::string map_path;
    float map_scale = 1.0f;
    SceneState state = CBE_SCENE_NEW;
    SpriteManager sprites;

    Scene(){};
    ~Scene();
    std::shared_ptr<Tilemap> GetTilemap() { return this->tilemap; };
    bool HasTilemap() { return this->tilemap != nullptr; };
    void NotifyLoaded();
    void NotifyUnloaded(bool);

  private:
    std::shared_ptr<Tilemap> tilemap = nullptr;
};

class SceneManager {
  public:
    std::shared_ptr<Scene> current_scene;

    SceneManager(){};
    ~SceneManager();
    bool IsCurrentSceneActive() { return this->current_scene != nullptr && this->current_scene->state == CBE_SCENE_ACTIVE; };
    bool LoadScene(const std::string &);
    static std::string GetScenePath(const std::string &);

  private:
    std::string scene_path;
    std::list<std::shared_ptr<Scene>> loaded_scenes;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    void UnloadCurrentScene();
};
}
#endif