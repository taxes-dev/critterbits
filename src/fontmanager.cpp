#include <cb/critterbits.hpp>
#include <SDL_ttf.h>

namespace Critterbits {

void FontManager::CleanUp() {
    for (auto it = this->fonts.begin(); it != this->fonts.end();) {
        if (it->second.unique()) {
            it = this->fonts.erase(it);
        } else {
            it++;
        }
    }
}

FontManager & FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

std::shared_ptr<TTF_Font> FontManager::GetFont(const std::string & asset_path, int pt_size, const std::string & relative_to_file) {
    std::string final_path{asset_path};
    if (!relative_to_file.empty()) {
        final_path = ResourceLoader::StripAssetNameFromPath(relative_to_file) + PATH_SEP + asset_path;
    }
    std::string key_name{final_path + ":" + std::to_string(pt_size)};
    auto it = this->fonts.find(key_name);
    if (it == this->fonts.end()) {
        LOG_INFO("FontManager::GetFont attempting to load " + final_path + " at size " + std::to_string(pt_size) + " pt");
        std::shared_ptr<TTF_Font> font_ptr = Engine::GetInstance().GetResourceLoader()->GetFontResource(final_path, pt_size);
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

std::shared_ptr<TTF_Font> FontManager::GetNamedFont(const std::string & name) {
    auto it = this->named_fonts.find(name);
    if (it != this->named_fonts.end()) {
        return this->GetFont(it->second.font_path, it->second.pt_size);
    } else {
        LOG_ERR("FontManager::GetNamedFont no font named " + name);
        return nullptr;
    }
}

void FontManager::RegisterNamedFont(const std::string & name, const std::string & font_path, int pt_size) {
    if (name.empty() || font_path.empty() || pt_size < CB_FONT_MIN_SIZE || pt_size > CB_FONT_MAX_SIZE) {
        LOG_ERR("FontManager::RegisterNamedFont invalid font data");
    }
    auto it = this->named_fonts.find(name);
    if (it == this->named_fonts.end()) {
        CB_NamedFont named_font{name, font_path, pt_size};
        this->named_fonts.insert(std::make_pair(name, named_font));
    } else {
        LOG_ERR("FontManager::RegisterNamedFont already defined a named font called " + name);
    }
}
}