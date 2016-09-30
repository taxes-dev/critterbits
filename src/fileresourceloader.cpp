#include <cassert>
#include <fstream>
#include <memory>
#include <sys/stat.h>

#include <cb/critterbits.hpp>
#include <SDL_image.h>
#include <SDL_ttf.h>

namespace Critterbits {
std::shared_ptr<TTF_FontWrapper> FileResourceLoader::GetFontResource(const std::string & asset_path, int pt_size) const {
    std::string font_path = this->res_path.base_path + asset_path;
    std::shared_ptr<TTF_FontWrapper> wrapper = std::make_shared<TTF_FontWrapper>();
    wrapper->font = TTF_OpenFont(font_path.c_str(), pt_size);
    if (wrapper->font == nullptr) {
        LOG_SDL_ERR("FileResourceLoader::GetFontResource unable to load font " + font_path);
        return nullptr;
    }
    return std::move(wrapper);
}

std::shared_ptr<SDL_Texture> FileResourceLoader::GetImageResource(const std::string & asset_path) const {
    std::string texture_path = this->res_path.base_path + asset_path;
    SDL_Texture * texture = IMG_LoadTexture(Engine::GetInstance().GetRenderer(), texture_path.c_str());
    if (texture == nullptr) {
        LOG_SDL_ERR("FileResourceLoader::GetImageResource unable to load image to texture " + texture_path);
        return nullptr;
    }
    std::shared_ptr<SDL_Texture> texture_ptr{texture, [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
    return std::move(texture_ptr);
}

std::shared_ptr<SDL_Surface> FileResourceLoader::GetImageResourceAsSurface(const std::string & asset_path) const {
    std::string surface_path = this->res_path.base_path + asset_path;
    SDL_Surface * surface = IMG_Load(surface_path.c_str());
    if (surface == nullptr) {
        LOG_SDL_ERR("FileResourceLoader::GetImageResourceAsSurface unable to load image " + surface_path);
        return nullptr;
    }
    std::shared_ptr<SDL_Surface> surface_ptr{surface, [](SDL_Surface * surface) { SDLx::SDL_CleanUp(surface); }};
    return std::move(surface_ptr);
}

bool FileResourceLoader::GetTextResourceContents(const std::string & asset_path, std::string ** text_content) const {
    std::ifstream ifs;

    ifs.open(this->res_path.base_path + asset_path, std::ifstream::in);
    if (ifs.good()) {
        *text_content = new std::string{};
        ifs.seekg(0, std::ios::end);  
        (*text_content)->reserve(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        (*text_content)->assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        return true;
    }
    return false;
}

std::shared_ptr<std::istream> FileResourceLoader::OpenTextResource(const std::string & asset_path) const {
    std::shared_ptr<std::ifstream> ifs = std::make_shared<std::ifstream>();
    ifs->open(this->res_path.base_path + asset_path, std::ifstream::in);
    if (!ifs->good()) {
        LOG_ERR("FileResourceLoader::OpenTextResource unable to read from text resource " + asset_path);
    }
    return std::move(ifs);
}

bool FileResourceLoader::ResourceExists(const std::string & asset_path) const {
    std::string file_path{this->res_path.base_path + asset_path};
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}
}