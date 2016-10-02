#include <cb/critterbits.hpp>
#include <SDL_image.h>

namespace {
    Critterbits::entity_id_t next_created_texture_id = CB_ENTITY_ID_FIRST;
}
namespace Critterbits {
TextureManager::TextureManager() {
    if (!TestBitMask<int>(IMG_Init(IMG_INIT_PNG), IMG_INIT_PNG)) {
        LOG_SDL_ERR("Engine::Engine IMG_Init");
    } else {
        this->initialized = true;
    }
}

TextureManager::~TextureManager() {
    for (auto & texture : this->textures) {
        texture.second.reset();
    }
    IMG_Quit();
}

void TextureManager::CleanUp() {
    for (auto it = this->textures.begin(); it != this->textures.end();) {
        if (it->second.unique()) {
            it = this->textures.erase(it);
        } else {
            it++;
        }
    }
}

std::shared_ptr<SDL_Texture> TextureManager::CreateTargetTexture(int w, int h, float scale, TextureCreateFunction func) {
    LOG_INFO("TextureManager::CreateTargetTexture creating new ad hoc texture " + std::to_string(w) + "x" + std::to_string(h));
    SDL_Renderer * renderer = Engine::GetInstance().GetRenderer();
    SDL_Texture * new_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

    // set render target to the new texture
    float original_scale_x, original_scale_y;
    SDL_BlendMode original_blend_mode;
    SDL_RenderGetScale(renderer, &original_scale_x, &original_scale_y);
    SDL_GetRenderDrawBlendMode(renderer, &original_blend_mode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // clear texture to transparent
    SDL_SetRenderTarget(renderer, new_texture);
    SDL_SetTextureBlendMode(new_texture, SDL_BLENDMODE_BLEND);
    SDL_RenderSetScale(renderer, scale, scale);

    func(renderer, new_texture);

    // reset render target
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawBlendMode(renderer, original_blend_mode);
    SDL_RenderSetScale(renderer, original_scale_x, original_scale_y);

    std::shared_ptr<SDL_Texture> texture_ptr{new_texture, [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
    this->textures.insert(std::make_pair(":" + std::to_string(next_created_texture_id++), texture_ptr));
    LOG_INFO("TextureManager::CreateTargetTexture texture created");
    return std::move(texture_ptr);
}

std::shared_ptr<SDL_Texture> TextureManager::GetTexture(const std::string & asset_path, const std::string & relative_to_file) {
    if (this->loader == nullptr) {
        LOG_ERR("TextureManager::GetTexture called before resource loader set (programming error?)");
        return nullptr;
    }
    std::string final_path{asset_path};
    if (!relative_to_file.empty()) {
        final_path = ResourceLoader::StripAssetNameFromPath(relative_to_file) + PATH_SEP_STR + asset_path;
    }
    auto it = this->textures.find(final_path);
    if (it == this->textures.end()) {
        LOG_INFO("TextureManager::GetTexture attempting to load " + final_path);
        std::shared_ptr<SDL_Texture> texture_ptr = this->loader->GetImageResource(final_path);
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

void TextureManager::SetResourceLoader(std::shared_ptr<ResourceLoader> resource_loader) {
    this->loader = std::move(resource_loader); 
}
}