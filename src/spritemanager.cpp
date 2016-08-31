#include <critterbits.h>

namespace Critterbits {
    bool SpriteManager::LoadSprite(std::string & sprite_name) {
        return true;
    }

void SpriteManager::SetAssetPath(std::string & asset_path) {
    this->sprite_path = asset_path + CB_SPRITE_PATH + PATH_SEP;
}

}