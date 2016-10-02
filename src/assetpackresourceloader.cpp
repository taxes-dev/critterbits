#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#ifdef _WIN32
#include <netinet/in.h>
#else
#include <arpa/inet.h>
#endif

#include <cb/critterbits.hpp>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <zstd.h>

namespace Critterbits {
namespace {
void read_header(std::ifstream & ifs, AssetPack::CB_AssetPackHeader * header) {
    ifs.read(reinterpret_cast<char *>(header), sizeof(AssetPack::CB_AssetPackHeader));
    header->flags = ntohl(header->flags);
    header->table_pos = ntohl(header->table_pos);
    header->first_resource_pos = ntohl(header->first_resource_pos);
}

void read_dict_entry(std::ifstream & ifs, AssetPack::CB_AssetDictEntry * entry) {
    ifs.read(reinterpret_cast<char *>(entry), sizeof(AssetPack::CB_AssetDictEntry));
    entry->index = ntohl(entry->index);
    entry->pos = ntohl(entry->pos);
    entry->length = ntohl(entry->length);
}
}

AssetPackResourceLoader::AssetPackResourceLoader(const BaseResourcePath & res_path) : ResourceLoader(res_path) {
    // extract the header and the table of resources from the pack
    this->pack = std::unique_ptr<std::ifstream>{new std::ifstream{res_path.base_path, std::ifstream::binary}};
    if (this->pack->good()) {
        read_header(*this->pack, &this->header);
        this->compressed = TestBitMask<unsigned int>(this->header.flags, CB_ASSETPACK_FLAGS_COMPRESSED);
        this->pack->seekg(this->header.table_pos);

        while (!this->pack->eof()) {
            AssetPack::CB_AssetDictEntry entry;
            read_dict_entry(*this->pack, &entry);
            this->dict.insert(std::make_pair(entry.name, entry));
        }
        this->pack->clear();
    } else {
        LOG_ERR("AssetPackResourceLoader unable to open asset pack " + res_path.base_path);
    }
}

std::shared_ptr<TTF_FontWrapper> AssetPackResourceLoader::GetFontResource(const std::string & asset_path, int pt_size) const {
    auto it = this->dict.find(asset_path);
    if (it != this->dict.end()) {
        LOG_INFO("AssetPackResourceLoader::GetFontResource loading asset " + asset_path + " at position " +
                 std::to_string(it->second.pos) + " with length " + std::to_string(it->second.length));
        std::shared_ptr<TTF_FontWrapper> wrapper = std::make_shared<TTF_FontWrapper>();
        wrapper->buffer = new char[it->second.length];
        this->pack->clear();
        this->pack->seekg(it->second.pos);
        this->pack->read(wrapper->buffer, it->second.length);
        SDL_RWops * rwops = SDL_RWFromMem(wrapper->buffer, it->second.length);
        wrapper->font = TTF_OpenFontRW(rwops, 1, pt_size);
        if (wrapper->font == nullptr) {
            LOG_SDL_ERR("AssetPackResourceLoader::GetFontResource unable to load font " + asset_path);
        } else {
            return std::move(wrapper);
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