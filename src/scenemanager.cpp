#include <string>

#include "cbfiles.h"
#include "cbscene.h"

namespace Critterbits {
bool SceneManager::LoadScene(const std::string & scene_name) { return true; }

void SceneManager::set_asset_path(const std::string & asset_path) {
    this->scene_path = asset_path + CB_SCENE_PATH + PATH_SEP;
}
}