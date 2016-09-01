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

typedef enum { CBE_NEW, CBE_ACTIVE, CBE_INACTIVE, CBE_UNLOADED } SceneState;

class Scene {
  public:
    bool persistent = false;
    std::string scene_name;
    std::string map_path;
    float map_scale = 1.0f;
    SceneState state = CBE_NEW;
    SpriteManager sprites;

    Scene(){};
    ~Scene();
    Tilemap * GetTilemap() { return this->tilemap; };
    void NotifyLoaded();
    void NotifyUnloaded();

  private:
    Tilemap * tilemap = nullptr;
};

class SceneManager {
  public:
    std::shared_ptr<Scene> current_scene;

    SceneManager(){};
    bool LoadScene(const std::string &);
    static std::string GetScenePath(const std::string &);

  private:
    std::string asset_path; // FIXME:reduce copies of this
    std::string scene_path;
    std::list<std::shared_ptr<Scene>> loaded_scenes;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    void UnloadCurrentScene();
};
}
#endif