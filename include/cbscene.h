#pragma once
#ifndef CBSCENE_H
#define CBSCENE_H

#include <list>
#include <string>

#define CB_SCENE_PATH "scenes"
#define CB_FIRST_SCENE "startup"
#define CB_SCENE_EXT ".yml"

namespace Critterbits {

class Scene {
  public:
    bool persistent = false;
    std::string scene_name;

    Scene(){};
    void NotifyLoaded();
    void NotifyUnloaded();
};

class SceneManager {
  public:
    Scene * current_scene = nullptr;

    SceneManager(){};
    bool LoadScene(const std::string &);
    void set_asset_path(const std::string &);

  private:
    std::string scene_path;
    std::list<Scene> loaded_scenes;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
    void UnloadCurrentScene();
};
}
#endif