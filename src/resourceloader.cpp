#include <cassert>
#include <fstream>
#include <memory>

#include <critterbits.hpp>

namespace Critterbits {
    std::shared_ptr<ResourceLoader> ResourceLoader::GetResourceLoader(const BaseResourcePath & base_path) {
        if (base_path.source == ResourceSource::File) {
            return std::make_shared<FileResourceLoader>(base_path);
        } else {
            assert(false);
            // TODO
            return nullptr;
        }
    }

    std::shared_ptr<std::istream> FileResourceLoader::OpenTextResource(const std::string & asset_path) const {
        std::shared_ptr<std::ifstream> ifs = std::make_shared<std::ifstream>();
        ifs->open(this->res_path.base_path + asset_path, std::ifstream::in);
        if (!ifs->good()) {
            LOG_ERR("FileResourceLoader::OpenTextResource unable to read from text resource " + asset_path);
        }
        return std::move(ifs);
    }
}