#include <list>
#include <memory>
#include <string>

#include <critterbits.h>

namespace Critterbits {

/*
 * Support functions for SceneManager::ReloadConfiguraLoadScenetion()
 */
/*namespace {
void map_parser(void * context, const std::string & value) {
    Scene * scene = static_cast<Scene *>(context);
    scene->map_path = SceneManager::GetScenePath(value);
}

void map_scale_parser(void * context, const std::string & value) {
    static_cast<Scene *>(context)->map_scale = YamlParser::ToFloat(value);
}

void persistent_parser(void * context, const std::string & value) {
    static_cast<Scene *>(context)->persistent = YamlParser::ToBool(value);
}

void sprites_parser(void * context, std::list<std::string> & values) {
    for (auto & sprite_name : values) {
        static_cast<Scene *>(context)->sprites.QueueSprite(sprite_name);
    }
}

YamlSequenceParserCollection scene_seq_parsers = {{"sprites", sprites_parser}};

YamlValueParserCollection scene_val_parsers = {
    {"map", map_parser}, {"map_scale", map_scale_parser}, {"persistent", persistent_parser}};
}*/
/*
 * End support functions
 */

SceneManager::~SceneManager() {
    this->current_scene = nullptr;
    for (auto & scene : this->loaded_scenes) {
        scene->NotifyUnloaded(true);
    }
}

std::string SceneManager::GetScenePath(const std::string & asset_name) {
    return Engine::GetInstance().config->asset_path + CB_SCENE_PATH + PATH_SEP + asset_name;
}

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
            scene->NotifyLoaded();
            this->current_scene = scene;
            break;
        }
    }

    // if we didn't have it loaded, fetch from disk
    if (this->current_scene == nullptr) {
        std::shared_ptr<Scene> new_scene = std::make_shared<Scene>();
        new_scene->scene_name = scene_name;

        std::string * scene_content = nullptr;
        std::string scene_path = SceneManager::GetScenePath(scene_name + CB_SCENE_EXT);
        if (!ReadTextFile(scene_path, &scene_content)) {
            LOG_ERR("SceneManager::LoadScene unable to load scene from " + scene_path);
            return false;
        }

        /*YamlParser parser;
        parser.sequence_parsers = scene_seq_parsers;
        parser.value_parsers = scene_val_parsers;
        parser.Parse(new_scene.get(), *scene_content);
        delete scene_content;*/

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