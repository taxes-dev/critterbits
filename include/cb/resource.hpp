#pragma once
#ifndef CBRESOURCE_HPP
#define CBRESOURCE_HPP

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>

#include <SDL.h>

#ifdef _WIN32
const char PATH_SEP = '\\';
#define PATH_SEP_STR "\\"
#else
const char PATH_SEP = '/';
#define PATH_SEP_STR "/"
#endif

namespace Critterbits {
enum class ResourceSource { File, Pack };

typedef struct BaseResourcePath {
    std::string base_path;
    ResourceSource source{ResourceSource::File};
} BaseResourcePath;

class ResourceLoader {
  public:
    virtual ~ResourceLoader(){};
    static std::shared_ptr<ResourceLoader> GetResourceLoader(const BaseResourcePath &);
    virtual std::shared_ptr<SDL_Texture> GetImageResource(const std::string &) const = 0;
    virtual bool GetTextResourceContents(const std::string &, std::string **) const = 0;
    virtual std::shared_ptr<std::istream> OpenTextResource(const std::string &) const = 0;
    virtual bool ResourceExists(const std::string &) const = 0;
    static std::string StripAssetNameFromPath(const std::string &);

  protected:
    BaseResourcePath res_path;

    ResourceLoader(const BaseResourcePath & res_path) : res_path(res_path){};
};

class FileResourceLoader : public ResourceLoader {
  public:
    FileResourceLoader(const BaseResourcePath & res_path) : ResourceLoader(res_path){};

    std::shared_ptr<SDL_Texture> GetImageResource(const std::string &) const;
    bool GetTextResourceContents(const std::string &, std::string **) const;
    std::shared_ptr<std::istream> OpenTextResource(const std::string &) const;
    bool ResourceExists(const std::string &) const;
};

typedef std::function<void(SDL_Renderer *, SDL_Texture *)> TextureCreateFunction;

class TextureManager {
  public:
    static TextureManager & GetInstance();

    void CleanUp();
    std::shared_ptr<SDL_Texture> CreateTargetTexture(int, int, float, TextureCreateFunction);
    std::shared_ptr<SDL_Texture> GetTexture(const std::string &, const std::string & = "");

  private:
    std::map<std::string, std::shared_ptr<SDL_Texture>> textures;

    TextureManager(){};
    TextureManager(const TextureManager &) = delete;
    TextureManager(TextureManager &&) = delete;
};
}
#endif