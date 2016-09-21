#include <cb/critterbits.hpp>
#include <SDL_image.h>

namespace Critterbits {

void TextureManager::CleanUp() {
    for (auto it = this->textures.begin(); it != this->textures.end();) {
        if (it->second.unique()) {
            it = this->textures.erase(it);
        } else {
            it++;
        }
    }
}

TextureManager & TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

std::shared_ptr<SDL_Texture> TextureManager::GetTexture(const std::string & asset_path, const std::string & relative_to_file) {
    std::string final_path{asset_path};
    if (!relative_to_file.empty()) {
        final_path = ResourceLoader::StripAssetNameFromPath(relative_to_file) + PATH_SEP + asset_path;
    }
    auto it = this->textures.find(final_path);
    if (it == this->textures.end()) {
        LOG_INFO("TextureManager::GetTexture attempting to load " + final_path);
        std::shared_ptr<SDL_Texture> texture_ptr = Engine::GetInstance().GetResourceLoader()->GetImageResource(final_path);
        if (texture_ptr == nullptr) {
            LOG_ERR("TextureManager::GetTexture unable to load image to texture");
            // push the bad image on to the map to prevent infinite attempts
            this->textures.insert(std::make_pair(final_path, nullptr));
        } else {
            this->textures.insert(std::make_pair(final_path, texture_ptr));
            return std::move(texture_ptr);
        }
    } else {
        return it->second;
    }
    return nullptr;
}


}