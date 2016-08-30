#pragma once
#ifndef CBSCENE_H
#define CBSCENE_H

#include <list>
#include <memory>
#include <string>

#include "cbtilemap.h"

#define CB_SCENE_PATH "scenes"
#define CB_FIRST_SCENE "startup"
#define CB_SCENE_EXT ".yml"

namespace Critterbits {

class Scene {
  public:
    bool persistent = false;
    std::string scene_name;
    std::string map_path;
    std::string scene_path;
    float map_scale = 1.0f;

    Scene(){};
    ~Scene();
    void NotifyLoaded();
    void NotifyUnloaded();
    SDL_Texture * GetMapTexture() const {
        if (this->tilemap != nullptr)
            return this->tilemap->GetMapTexture();
        return nullptr;
    };

  private:
    Tilemap * tilemap = nullptr;
};

class SceneManager {
  public:
    std::shared_ptr<Scene> current_scene;

    SceneManager(){};
    bool LoadScene(const std::string &);
    void SetAssetPath(const std::string &);

  private:
    std::string scene_path;
    std::list<std::shared_ptr<Scene>> loaded_scenes;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    void UnloadCurrentScene();
};
}
#endif