#include <sys/stat.h>

#include <cb/critterbits.hpp>

namespace Critterbits {
std::shared_ptr<ResourceLoader> ResourceLoader::GetResourceLoader(const BaseResourcePath & base_path) {
    if (base_path.source == ResourceSource::File) {
        return std::make_shared<FileResourceLoader>(base_path);
    } else if (base_path.source == ResourceSource::AssetPack) {
        return std::make_shared<AssetPackResourceLoader>(base_path);
    }
    return nullptr;
}

FileType ResourceLoader::IsFileOrDirectory(const std::string & file_name) {
    struct stat buffer;
    if (stat(file_name.c_str(), &buffer) == 0) {
        if (buffer.st_mode & S_IFDIR) {
            return FileType::Directory;
        } else if (buffer.st_mode & S_IFREG) {
            return FileType::File;
        }
    }
    return FileType::Invalid;
}

std::string ResourceLoader::StripAssetNameFromPath(const std::string & asset_path) {
    int index_a = asset_path.find_last_of('\\');
    int index_b = asset_path.find_last_of('/');
    return asset_path.substr(0, std::max(index_a, index_b));
}
}