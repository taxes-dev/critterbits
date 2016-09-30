#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

#include <cb/critterbits.hpp>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <zstd.h>

namespace Critterbits {
AssetPackResourceLoader::AssetPackResourceLoader(const BaseResourcePath & res_path) : ResourceLoader(res_path) {
    // extract the header and the table of resources from the pack
    this->pack = std::unique_ptr<std::ifstream>{new std::ifstream{res_path.base_path, std::ifstream::binary}};
    if (this->pack->good()) {
        // TODO: account for endianness
        this->pack->read(reinterpret_cast<char *>(&this->header), sizeof(AssetPack::CB_AssetPackHeader));
        this->compressed = TestBitMask<unsigned int>(this->header.flags, CB_ASSETPACK_FLAGS_COMPRESSED);
        this->pack->seekg(this->header.table_pos);

        while (!this->pack->eof()) {
            AssetPack::CB_AssetDictEntry entry;
            this->pack->read(reinterpret_cast<char *>(&entry), sizeof(AssetPack::CB_AssetDictEntry));
            this->dict.insert(std::make_pair(entry.name, entry));
        }
        this->pack->clear();
    } else {
        LOG_ERR("AssetPackResourceLoader unable to open asset pack " + res_path.base_path);
    }
}

std::shared_ptr<TTF_Font> AssetPackResourceLoader::GetFontResource(const std::string & asset_path, int pt_size) const {
    auto it = this->dict.find(asset_path);
    if (it != this->dict.end()) {
        LOG_INFO("AssetPackResourceLoader::GetFontResource loading asset " + asset_path + " at position " +
                 std::to_string(it->second.pos) + " with length " + std::to_string(it->second.length));
        char * buffer = new char[it->second.length];
        this->pack->clear();
        this->pack->seekg(it->second.pos);
        this->pack->read(buffer, it->second.length);
        SDL_RWops * rwops = SDL_RWFromMem(buffer, it->second.length);
        TTF_Font * font = TTF_OpenFontRW(rwops, 1, pt_size);
        //FIXME: nice memory leak... apparently TTF_OpenFontRW is pointing at the underlying buffer rather than copying it
        //delete[] buffer;
        if (font == nullptr) {
            LOG_SDL_ERR("AssetPackResourceLoader::GetFontResource unable to load font " + asset_path);
        } else {
            std::shared_ptr<TTF_Font> font_ptr{font, [](TTF_Font * font) {
                                                   if (font != nullptr) {
                                                       TTF_CloseFont(font);
                                                   }
                                               }};
            return std::move(font_ptr);
        }
    } else {
        LOG_ERR("AssetPackResourceLoader::GetFontResource asset not found " + asset_path);
    }
    return nullptr;
}

std::shared_ptr<SDL_Texture> AssetPackResourceLoader::GetImageResource(const std::string & asset_path) const {
    auto it = this->dict.find(asset_path);
    if (it != this->dict.end()) {
        LOG_INFO("AssetPackResourceLoader::GetImageResource loading asset " + asset_path + " at position " +
                 std::to_string(it->second.pos) + " with length " + std::to_string(it->second.length));
        char * buffer = new char[it->second.length];
        this->pack->clear();
        this->pack->seekg(it->second.pos);
        this->pack->read(buffer, it->second.length);
        SDL_RWops * rwops = SDL_RWFromMem(buffer, it->second.length);
        SDL_Texture * texture = IMG_LoadTextureTyped_RW(Engine::GetInstance().GetRenderer(), rwops, 1, "PNG");
        delete[] buffer;
        if (texture == nullptr) {
            LOG_SDL_ERR("AssetPackResourceLoader::GetImageResource unable to load image to texture " + asset_path);
        } else {
            std::shared_ptr<SDL_Texture> texture_ptr{texture,
                                                     [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
            return std::move(texture_ptr);
        }
    } else {
        LOG_ERR("AssetPackResourceLoader::GetImageResource asset not found " + asset_path);
    }
    return nullptr;
}

std::shared_ptr<SDL_Surface> AssetPackResourceLoader::GetImageResourceAsSurface(const std::string & asset_path) const {
    auto it = this->dict.find(asset_path);
    if (it != this->dict.end()) {
        LOG_INFO("AssetPackResourceLoader::GetImageResource loading asset " + asset_path + " at position " +
                 std::to_string(it->second.pos) + " with length " + std::to_string(it->second.length));
        char * buffer = new char[it->second.length];
        this->pack->clear();
        this->pack->seekg(it->second.pos);
        this->pack->read(buffer, it->second.length);
        SDL_RWops * rwops = SDL_RWFromMem(buffer, it->second.length);
        SDL_Surface * surface = IMG_LoadTyped_RW(rwops, 1, "PNG");
        delete[] buffer;
        if (surface == nullptr) {
            LOG_SDL_ERR("FileResourceLoader::GetImageResourceAsSurface unable to load image " + asset_path);
        } else {
            std::shared_ptr<SDL_Surface> surface_ptr{surface,
                                                     [](SDL_Surface * surface) { SDLx::SDL_CleanUp(surface); }};
            return std::move(surface_ptr);
        }
    } else {
        LOG_ERR("AssetPackResourceLoader::GetImageResourceAsSurface asset not found " + asset_path);
    }
    return nullptr;
}

bool AssetPackResourceLoader::GetTextResourceContents(const std::string & asset_path,
                                                      std::string ** text_content) const {
    auto it = this->dict.find(asset_path);
    if (it == this->dict.end()) {
        return false;
    }
    const AssetPack::CB_AssetDictEntry & entry = it->second;
    LOG_INFO("AssetPackResourceLoader::GetTextResourceContents loading asset " + asset_path + " at pack position " +
             std::to_string(entry.pos) + " with length " + std::to_string(entry.length));
    this->pack->clear();
    this->pack->seekg(entry.pos);

    if (entry.length < 1) {
        *text_content = new std::string{};
        return true;
    } else if (this->compressed) {
        // TODO
    } else {
        char * buffer = new char[entry.length];
        this->pack->read(buffer, entry.length);
        *text_content = new std::string(buffer, this->pack->gcount());
        delete[] buffer;
    }
    return true;
}

std::shared_ptr<std::istream> AssetPackResourceLoader::OpenTextResource(const std::string & asset_path) const {
    std::string * contents = nullptr;
    if (this->GetTextResourceContents(asset_path, &contents)) {
        std::shared_ptr<std::istringstream> stream = std::make_shared<std::istringstream>(*contents);
        delete contents;
        return std::move(stream);
    }
    return nullptr;
}

bool AssetPackResourceLoader::ResourceExists(const std::string & asset_path) const {
    return this->dict.find(asset_path) != this->dict.end();
}

}