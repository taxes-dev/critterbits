#include <cb/critterbits.hpp>
#include <SDL_ttf.h>

namespace Critterbits {
FontManager::FontManager() {
    if (TTF_Init() != 0) {
        LOG_SDL_ERR("FontManager::FontManager TTF_Init");
    } else {
        this->initialized = true;
    }
}

FontManager::~FontManager() {
    for (auto & font : this->fonts) {
        font.second.reset();
    }
    TTF_Quit();
}

void FontManager::CleanUp() {
    for (auto it = this->fonts.begin(); it != this->fonts.end();) {
        if (it->second.unique()) {
            it = this->fonts.erase(it);
        } else {
            it++;
        }
    }
}

std::shared_ptr<TTF_FontWrapper> FontManager::GetFont(const std::string & asset_path, int pt_size, const std::string & relative_to_file) {
    if (this->loader == nullptr) {
        LOG_ERR("FontManager::GetFont called before resource loader set (programming error?)");
        return nullptr;
    }
    std::string final_path{asset_path};
    if (!relative_to_file.empty()) {
        final_path = ResourceLoader::StripAssetNameFromPath(relative_to_file) + PATH_SEP + asset_path;
    }
    std::string key_name{final_path + ":" + std::to_string(pt_size)};
    auto it = this->fonts.find(key_name);
    if (it == this->fonts.end()) {
        LOG_INFO("FontManager::GetFont attempting to load " + final_path + " at size " + std::to_string(pt_size) + " pt");
        std::shared_ptr<TTF_FontWrapper> font_ptr = this->loader->GetFontResource(final_path, pt_size);
        if (font_ptr == nullptr) {
            LOG_ERR("FontManager::GetFont unable to load font");
            // push the bad font on to the map to prevent infinite attempts
            this->fonts.insert(std::make_pair(key_name, nullptr));
        } else {
            LOG_INFO("FontManager::GetFont font successfully loaded");
            this->fonts.insert(std::make_pair(key_name, font_ptr));
            return std::move(font_ptr);
        }
    } else {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<TTF_FontWrapper> FontManager::GetNamedFont(const std::string & name) {
    auto it = this->named_fonts.find(name);
    if (it != this->named_fonts.end()) {
        return this->GetFont(it->second.font_path, it->second.pt_size);
    } else {
        LOG_ERR("FontManager::GetNamedFont no font named " + name);
        return nullptr;
    }
}

void FontManager::RegisterNamedFont(const CB_NamedFont & named_font) {
    if (named_font.name.empty() || named_font.font_path.empty() ||
        named_font.pt_size < CB_FONT_MIN_SIZE || named_font.pt_size > CB_FONT_MAX_SIZE) {
        LOG_ERR("FontManager::RegisterNamedFont invalid font data");
    }
    auto it = this->named_fonts.find(named_font.name);
    if (it == this->named_fonts.end()) {
        this->named_fonts.insert(std::make_pair(named_font.name, named_font));
    } else {
        LOG_ERR("FontManager::RegisterNamedFont already defined a named font called " + named_font.name);
    }
}

void FontManager::RegisterNamedFont(const std::string & name, const std::string & font_path, int pt_size) {
    CB_NamedFont named_font{name, font_path, pt_size};
    this->RegisterNamedFont(named_font);
}

void FontManager::SetResourceLoader(std::shared_ptr<ResourceLoader> resource_loader) {
    this->loader = std::move(resource_loader);
}

}