#include <memory>
#include <string>

#include <cb/critterbits.hpp>

namespace Critterbits {

SceneManager::~SceneManager() {
    this->current_scene = nullptr;
    for (auto & scene : this->loaded_scenes) {
        scene->NotifyUnloaded(true);
    }
}

std::string SceneManager::GetScenePath(const std::string & asset_name) const {
    return CB_SCENE_PATH PATH_SEP_STR + asset_name + CB_SCENE_EXT;
}

bool SceneManager::LoadScene(const std::string & scene_name) {
    // nothing to do if trying to load the same scene that's loaded
    if (this->IsCurrentSceneActive() && this->current_scene->scene_name == scene_name) {
        return true;
    }

    // unload the current scene before proceeding
    this->UnloadCurrentScene();

    // look in the list of loaded scenes to see if we already have an instance of the
    // requested scene
    for (auto & scene : this->loaded_scenes) {
        if (scene->scene_name == scene_name) {
            scene->NotifyLoaded();
            this->current_scene = scene;
            break;
        }
    }

    // if we didn't have it loaded, fetch from disk
    if (this->current_scene == nullptr) {
        std::shared_ptr<Scene> new_scene = std::make_shared<Scene>();
        new_scene->scene_name = scene_name;

        auto scene_file = Engine::GetInstance().GetResourceLoader()->OpenTextResource(this->GetScenePath(scene_name));
        Toml::TomlParser parser{scene_file};
        if (parser.IsReady()) {
            std::string map_path = parser.GetTableString("scene.map");
            if (!map_path.empty()) {
                new_scene->map_path = CB_SCENE_PATH PATH_SEP_STR + map_path;
            }
            new_scene->map_scale = parser.GetTableFloat("scene.map_scale", 1.0f);
            new_scene->persistent = parser.GetTableBool("scene.persistent");
            parser.IterateTableArray("sprite", [&new_scene](const Toml::TomlParser & table) {
                QueuedSprite qsprite{table.GetTableString("name"), table.GetTablePoint("at")};
                if (!qsprite.name.empty()) {
                    new_scene->sprites.QueueSprite(qsprite);
                }
            });
        } else {
            LOG_ERR("SceneManager::LoadScene unable to parse scene file " + scene_path);
            return false;
        }

        this->current_scene = new_scene;
        this->loaded_scenes.push_back(std::move(new_scene));
    }

    // notify the newly loaded scene that it is active
    this->current_scene->NotifyLoaded();

    return true;
}

void SceneManager::UnloadCurrentScene() {
    if (this->current_scene != nullptr) {
        // notify the current scene that it's being unloaded
        this->current_scene->NotifyUnloaded(!this->current_scene->persistent);

        // remove scene from loaded scenes if it's not marked persistent
        if (!this->current_scene->persistent) {
            for (auto it = this->loaded_scenes.begin(); it != this->loaded_scenes.end(); it++) {
                if (*it == this->current_scene) {
                    this->loaded_scenes.erase(it);
                    break;
                }
            }
        }

        // clear current scene
        this->current_scene = nullptr;
    }
}
}