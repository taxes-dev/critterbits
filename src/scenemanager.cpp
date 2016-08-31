#include <list>
#include <memory>
#include <string>

#include <critterbits.h>

namespace Critterbits {

/*
 * Support functions for SceneManager::ReloadConfiguraLoadScenetion()
 */
static void map_parser(void * context, const char * value, const size_t size) {
    Scene * scene = static_cast<Scene *>(context);
    scene->map_path = scene->scene_path + std::string(value, size);
}

static void map_scale_parser(void * context, const char * value, const size_t size) {
    static_cast<Scene *>(context)->map_scale = YamlParser::ToFloat(value);
}

static void persistent_parser(void * context, const char * value, const size_t size) {
    static_cast<Scene *>(context)->persistent = YamlParser::ToBool(value);
}

static YamlParserCollection scene_parsers = {
    {"map", map_parser}, {"map_scale", map_scale_parser}, {"persistent", persistent_parser}};
/*
 * End support functions
 */

bool SceneManager::LoadScene(const std::string & scene_name) {
    // nothing to do if trying to load the same scene that's loaded
    if (this->current_scene != nullptr && this->current_scene->scene_name == scene_name) {
        return true;
    }

    // unload the current scene before proceeding
    this->UnloadCurrentScene();

    // look in the list of loaded scenes to see if we already have an instance of the
    // requested scene
    for (auto & scene : this->loaded_scenes) {
        if (scene->scene_name == scene_name) {
            scene->state = CBE_ACTIVE;
            this->current_scene = scene;
            break;
        }
    }

    // if we didn't have it loaded, fetch from disk
    if (this->current_scene == nullptr) {
        std::shared_ptr<Scene> new_scene(new Scene());
        new_scene->scene_name = scene_name;
        new_scene->scene_path = this->scene_path;

        std::string * scene_content = nullptr;
        if (!ReadTextFile(this->scene_path + scene_name + CB_SCENE_EXT, &scene_content)) {
            LOG_ERR("SceneManager::LoadScene unable to load scene from " + this->scene_path + scene_name +
                    CB_SCENE_EXT);
            return false;
        }

        YamlParser parser;
        parser.Parse(new_scene.get(), scene_parsers, *scene_content);
        delete scene_content;

        this->loaded_scenes.push_back(new_scene);
        this->current_scene = new_scene;
    }

    // notify the newly loaded scene that it is active
    this->current_scene->NotifyLoaded();

    return true;
}

void SceneManager::SetAssetPath(const std::string & asset_path) {
    this->scene_path = asset_path + CB_SCENE_PATH + PATH_SEP;
}

void SceneManager::UnloadCurrentScene() {
    if (this->current_scene != nullptr) {
        // notify the current scene that it's being unloaded
        this->current_scene->NotifyUnloaded();

        // remove scene from loaded scenes if it's not marked persistent
        if (this->current_scene->persistent) {
            this->current_scene->state = CBE_INACTIVE;
        } else {
            this->current_scene->state = CBE_UNLOADED;
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