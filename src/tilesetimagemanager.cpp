#include <SDL_image.h>
#include <critterbits.h>

namespace Critterbits {
TilesetImageManager & TilesetImageManager::GetInstance() {
    static TilesetImageManager instance;
    return instance;
}

std::shared_ptr<SDL_Texture> TilesetImageManager::GetTilesetImage(const std::string & tmx_path,
                                                                  const std::string & relative_path) {
    std::string abs_path = GetExpandedPath(StripFileFromPath(tmx_path) + PATH_SEP + relative_path);
    if (abs_path.empty()) {
        LOG_INFO("TilesetImageManager::GetTilesetImage unable to parse provided tileset image paths");
        return nullptr;
    }
    auto it = this->tileset_images.find(abs_path);
    if (it == this->tileset_images.end()) {
        LOG_INFO("TilesetImageManager::GetTilesetImage attempting to load tileset " + abs_path);
        SDL_Texture * tileset = IMG_LoadTexture(Engine::GetInstance().GetRenderer(), abs_path.c_str());
        if (tileset == nullptr) {
            LOG_SDL_ERR("TilesetImageManager::GetTilesetImage unable to load tileset to texture");
            // push the bad tileset on to the map to prevent infinite attempts
            this->tileset_images.insert(std::make_pair(abs_path, nullptr));
            return nullptr;
        }
        std::shared_ptr<SDL_Texture> tileset_ptr{tileset, [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
        this->tileset_images.insert(std::make_pair(abs_path, tileset_ptr));
        return std::move(tileset_ptr);
    } else {
        return it->second;
    }
    return nullptr;
}
}