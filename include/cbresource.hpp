#pragma once
#ifndef CBRESOURCE_HPP
#define CBRESOURCE_HPP

#include <iostream>
#include <memory>

namespace Critterbits {
    enum class ResourceSource { File, Pack };

    typedef struct BaseResourcePath {
        std::string base_path;
        ResourceSource source{ResourceSource::File};
    } BaseResourcePath;

    class ResourceLoader {
        public:
            virtual ~ResourceLoader() {};
            static std::shared_ptr<ResourceLoader> GetResourceLoader(const BaseResourcePath &);
            virtual std::shared_ptr<std::istream> OpenTextResource(const std::string &) const = 0;

        protected:
            BaseResourcePath res_path;

            ResourceLoader(const BaseResourcePath & res_path) : res_path(res_path) {};
    };

    class FileResourceLoader : public ResourceLoader {
        public:
            FileResourceLoader(const BaseResourcePath & res_path) : ResourceLoader(res_path) {};

            std::shared_ptr<std::istream> OpenTextResource(const std::string &) const;
    };
}
#endif