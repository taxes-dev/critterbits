#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

#include <cb/critterbits.hpp>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <zstd.h>

namespace Critterbits {
namespace {
Sint64 rwops_istream_seek(struct SDL_RWops * context, Sint64 offset, int whence) {
    std::istream * stream = static_cast<std::istream *>(context->hidden.unknown.data1);
    AssetPack::CB_AssetDictEntry * entry = static_cast<AssetPack::CB_AssetDictEntry *>(context->hidden.unknown.data2);

    if (whence == SEEK_SET) {
        stream->seekg(entry->pos + offset, std::ios::beg);
    } else if (whence == SEEK_CUR) {
        // FIXME: should we prevent from exiting the bounds of the current file?
        stream->seekg(offset, std::ios::cur);
    } else if (whence == SEEK_END) {
        stream->seekg(entry->pos + entry->length - offset, std::ios::beg);
    }

    if ( stream->fail()) {
        return -1;
    } else {
        return stream->tellg();
    }
}

size_t rwops_istream_read(SDL_RWops * context, void * ptr, size_t size, size_t maxnum) {
    if (size == 0) {
        return -1;
    }
    std::istream * stream = static_cast<std::istream *>(context->hidden.unknown.data1);
    AssetPack::CB_AssetDictEntry * entry = static_cast<AssetPack::CB_AssetDictEntry *>(context->hidden.unknown.data2);
    unsigned long remaining_length = (entry->pos + entry->length) - stream->tellg();
    if (size * maxnum > remaining_length) {
        maxnum = remaining_length / size;
    }
    stream->read(static_cast<char *>(ptr), size * maxnum);

    if (stream->bad()) {
        return 0 ;
    } else {
        return stream->gcount() / size;
    }
}

int rwops_istream_close(SDL_RWops * context) {
    if (context) {
        SDL_FreeRW(context);
    }
    return 0;
}
}
AssetPackResourceLoader::AssetPackResourceLoader(const BaseResourcePath & res_path) : ResourceLoader(res_path) {
    // extract the header and the table of resources from the pack
    this->pack = std::unique_ptr<std::ifstream>{new std::ifstream{res_path.base_path, std::ifstream::binary}};
    if (pack->good()) {
        // TODO: account for endianness
        pack->read(reinterpret_cast<char *>(&this->header), sizeof(AssetPack::CB_AssetPackHeader));
        this->compressed = TestBitMask<unsigned int>(this->header.flags, CB_ASSETPACK_FLAGS_COMPRESSED);
        pack->seekg(this->header.table_pos);

        while (!this->pack->eof()) {
            AssetPack::CB_AssetDictEntry entry;
            pack->read(reinterpret_cast<char *>(&entry), sizeof(AssetPack::CB_AssetDictEntry));
            this->dict.insert(std::make_pair(entry.name, entry));
        }
        this->pack->clear();
    } else {
        LOG_ERR("AssetPackResourceLoader unable to open asset pack " + res_path.base_path);
    }
}

std::shared_ptr<TTF_Font> AssetPackResourceLoader::GetFontResource(const std::string &, int) const { return nullptr; }

std::shared_ptr<SDL_Texture> AssetPackResourceLoader::GetImageResource(const std::string & asset_path) const {
    LOG_INFO("AssetPackResourceLoader::GetImageResource loading asset " + asset_path);
    SDL_RWops * rwops = this->GetSdlLoader(asset_path);
    SDL_Texture * texture = IMG_LoadTextureTyped_RW(Engine::GetInstance().GetRenderer(), rwops, 1, "PNG");
    if (texture == nullptr) {
        LOG_SDL_ERR("AssetPackResourceLoader::GetImageResource unable to load image to texture " + asset_path);
        return nullptr;
    }
    std::shared_ptr<SDL_Texture> texture_ptr{texture, [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
    return std::move(texture_ptr);
}

std::shared_ptr<SDL_Surface> AssetPackResourceLoader::GetImageResourceAsSurface(const std::string &) const {
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
    this->pack->seekg(entry.pos);

    if (entry.length < 1) {
        *text_content = new std::string{};
        return true;
    } else if (this->compressed) {
        // TODO
    } else {
        *text_content = new std::string(entry.length, '\0');
        std::copy_n((std::istreambuf_iterator<char>(*this->pack)), entry.length, std::back_inserter(**text_content));
    }
    return true;
}

std::shared_ptr<std::istream> AssetPackResourceLoader::OpenTextResource(const std::string & asset_path) const {
    std::string * contents = nullptr;
    if (this->GetTextResourceContents(asset_path, &contents)) {
        std::shared_ptr<std::istringstream> stream = std::make_shared<std::istringstream>(*contents);
        delete contents;
        // FIXME: would really love to know why I need to do this... (getline() does not work without)
        stream->ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return std::move(stream);
    }
    return nullptr;
}

bool AssetPackResourceLoader::ResourceExists(const std::string & asset_path) const {
    return this->dict.find(asset_path) != this->dict.end();
}

SDL_RWops * AssetPackResourceLoader::GetSdlLoader(const std::string & asset_path) const {
    SDL_RWops * ops = SDL_AllocRW();
    ops->seek = rwops_istream_seek;
    ops->read = rwops_istream_read;
    ops->write = nullptr;
    ops->close = rwops_istream_close;
    ops->hidden.unknown.data1 = const_cast<void *>(reinterpret_cast<const void *>(this->pack.get()));
    auto it = this->dict.find(asset_path);
    if (it != this->dict.end()) {
        ops->hidden.unknown.data2 = const_cast<void *>(reinterpret_cast<const void *>(&it->second));
    }
    return ops;
    }
}