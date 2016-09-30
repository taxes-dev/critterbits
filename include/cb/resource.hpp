#pragma once
#ifndef CBRESOURCE_HPP
#define CBRESOURCE_HPP

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include <SDL.h>

#include "assetpack.hpp"

#ifdef _WIN32
const char PATH_SEP = '\\';
#define PATH_SEP_STR "\\"
#else
const char PATH_SEP = '/';
#define PATH_SEP_STR "/"
#endif

#define CB_FONT_MIN_SIZE 6
#define CB_FONT_MAX_SIZE 72

// forward declaration from SDL_ttf.h
typedef struct _TTF_Font TTF_Font;

namespace Critterbits {
enum class FileType { File, Directory, Invalid };
enum class ResourceSource { File, AssetPack };

typedef struct BaseResourcePath {
    std::string base_path;
    ResourceSource source{ResourceSource::File};
} BaseResourcePath;

class ResourceLoader {
  public:
    virtual ~ResourceLoader(){};
    
    static std::shared_ptr<ResourceLoader> GetResourceLoader(const BaseResourcePath &);
    static FileType IsFileOrDirectory(const std::string &);

    virtual std::shared_ptr<TTF_Font> GetFontResource(const std::string &, int) const = 0;
    virtual std::shared_ptr<SDL_Texture> GetImageResource(const std::string &) const = 0;
    virtual std::shared_ptr<SDL_Surface> GetImageResourceAsSurface(const std::string &) const = 0;
    virtual bool GetTextResourceContents(const std::string &, std::string **) const = 0;
    virtual std::shared_ptr<std::istream> OpenTextResource(const std::string &) const = 0;
    virtual bool ResourceExists(const std::string &) const = 0;
    static std::string StripAssetNameFromPath(const std::string &);

  protected:
    BaseResourcePath res_path;

    ResourceLoader(const BaseResourcePath & res_path) : res_path(res_path){};
};

class AssetPackResourceLoader : public ResourceLoader {
  public:
    AssetPackResourceLoader(const BaseResourcePath &);

    std::shared_ptr<TTF_Font> GetFontResource(const std::string &, int) const;
    std::shared_ptr<SDL_Texture> GetImageResource(const std::string &) const;
    std::shared_ptr<SDL_Surface> GetImageResourceAsSurface(const std::string &) const;
    bool GetTextResourceContents(const std::string &, std::string **) const;
    std::shared_ptr<std::istream> OpenTextResource(const std::string &) const;
    bool ResourceExists(const std::string &) const;

  private:
    AssetPack::CB_AssetPackHeader header;
    std::map<std::string, AssetPack::CB_AssetDictEntry> dict;
    std::unique_ptr<std::ifstream> pack;
    bool compressed{false};
};

class FileResourceLoader : public ResourceLoader {
  public:
    FileResourceLoader(const BaseResourcePath & res_path) : ResourceLoader(res_path){};

    std::shared_ptr<TTF_Font> GetFontResource(const std::string &, int) const;
    std::shared_ptr<SDL_Texture> GetImageResource(const std::string &) const;
    std::shared_ptr<SDL_Surface> GetImageResourceAsSurface(const std::string &) const;
    bool GetTextResourceContents(const std::string &, std::string **) const;
    std::shared_ptr<std::istream> OpenTextResource(const std::string &) const;
    bool ResourceExists(const std::string &) const;
};

typedef std::function<void(SDL_Renderer *, SDL_Texture *)> TextureCreateFunction;

class TextureManager {
  public:
    TextureManager();
    ~TextureManager();

    void CleanUp();
    std::shared_ptr<SDL_Texture> CreateTargetTexture(int, int, float, TextureCreateFunction);
    std::shared_ptr<SDL_Texture> GetTexture(const std::string &, const std::string & = "");
    bool IsInitialized() const { return this->initialized; };
    void SetResourceLoader(std::shared_ptr<ResourceLoader>);

  private:
    bool initialized{false};
    std::map<std::string, std::shared_ptr<SDL_Texture>> textures;
    std::shared_ptr<ResourceLoader> loader;

    TextureManager(const TextureManager &) = delete;
    TextureManager(TextureManager &&) = delete;
};

typedef struct CB_NamedFont {
  std::string name;
  std::string font_path;
  int pt_size;

  CB_NamedFont() {};
  CB_NamedFont(const std::string & name, const std::string & font_path, int pt_size) : name(name), font_path(font_path), pt_size(pt_size) {};
} CB_NamedFont;

class FontManager {
  public:
    FontManager();
    ~FontManager();

    void CleanUp();
    std::shared_ptr<TTF_Font> GetFont(const std::string &, int, const std::string & = "");
    std::shared_ptr<TTF_Font> GetNamedFont(const std::string &);
    bool IsInitialized() const { return this->initialized; };
    void RegisterNamedFont(const CB_NamedFont &);
    void RegisterNamedFont(const std::string &, const std::string &, int);
    void SetResourceLoader(std::shared_ptr<ResourceLoader>);

  private:
    bool initialized{false};
    std::map<std::string, std::shared_ptr<TTF_Font>> fonts;
    std::map<std::string, CB_NamedFont> named_fonts;
    std::shared_ptr<ResourceLoader> loader; 

    FontManager(const FontManager &) = delete;
    FontManager(FontManager &&) = delete;
};
}
#endif