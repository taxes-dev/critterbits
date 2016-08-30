#include "cblogging.h"
#include "cbscene.h"

namespace Critterbits {

void Scene::NotifyLoaded() { LOG_INFO("Scene::NotifyLoaded scene was loaded: " + scene_name); }

void Scene::NotifyUnloaded() { LOG_INFO("Scene::NotifyUnloaded scene was unloaded: " + scene_name); }
}