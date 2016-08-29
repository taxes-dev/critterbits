#pragma once
#ifndef CBSCENE_H
#define CBSCENE_H

#include <string>

#define CB_SCENE_PATH "scenes"
#define CB_FIRST_SCENE "startup"
#define CB_SCENE_EXT ".yml"

namespace Critterbits {

class Scene {
  public:
    Scene(){};
};

class SceneManager {
  public:
    SceneManager(){};
    bool LoadScene(const std::string &);
    void set_asset_path(const std::string &);

  private:
    std::string scene_path;
    Scene * current_scene = nullptr;

    SceneManager(const SceneManager &) = delete;
    SceneManager(SceneManager &&) = delete;
};
}
#endif