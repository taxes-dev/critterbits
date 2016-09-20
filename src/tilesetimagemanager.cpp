#include <cb/critterbits.hpp>

namespace Critterbits {
TilesetImageManager & TilesetImageManager::GetInstance() {
    static TilesetImageManager instance;
    return instance;
}

std::shared_ptr<SDL_Texture> TilesetImageManager::GetTilesetImage(const std::string & tmx_path,
                                                                  const std::string & relative_path) {
    std::string asset_path = ResourceLoader::StripAssetNameFromPath(tmx_path) + PATH_SEP + relative_path;
    auto it = this->tileset_images.find(asset_path);
    if (it == this->tileset_images.end()) {
        LOG_INFO("TilesetImageManager::GetTilesetImage attempting to load tileset " + asset_path);
        std::shared_ptr<SDL_Texture> tileset_ptr = Engine::GetInstance().GetResourceLoader()->GetImageResource(asset_path);
        if (tileset_ptr == nullptr) {
            LOG_ERR("TilesetImageManager::GetTilesetImage unable to load tileset to texture");
            // push the bad tileset on to the map to prevent infinite attempts
            this->tileset_images.insert(std::make_pair(asset_path, nullptr));
            return nullptr;
        }
        this->tileset_images.insert(std::make_pair(asset_path, tileset_ptr));
        return std::move(tileset_ptr);
    } else {
        return it->second;
    }
    return nullptr;
}
}