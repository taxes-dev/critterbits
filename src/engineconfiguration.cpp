#include <string>
#include <limits.h>
#include <stdlib.h>

#include <cb/critterbits.hpp>

namespace Critterbits {

EngineConfiguration::EngineConfiguration(const std::string & source_path) {
    char * base_path = SDL_GetBasePath();

    std::string unexpanded_path;
    if (source_path.empty() || source_path[0] == '.' || source_path[0] != PATH_SEP) {
        // relative path
        unexpanded_path = std::string(base_path) + source_path;
    } else {
        // absolute path
        unexpanded_path = source_path;
    }

    SDL_free(base_path);

    LOG_INFO("EngineConfiguration unexpanded source path: " + unexpanded_path);

    std::string expanded_path = this->GetExpandedPath(unexpanded_path);
    if (!expanded_path.empty()) {
        BaseResourcePath res_path;
        FileType file_type = ResourceLoader::IsFileOrDirectory(expanded_path); 
        if (file_type == FileType::Directory) {
            this->asset_path = expanded_path + PATH_SEP_STR;
            LOG_INFO("EngineConfiguration expanded source path: " + this->asset_path);
            res_path.base_path = this->asset_path;
            res_path.source = ResourceSource::File;
        } else if (file_type == FileType::File) {
            this->asset_path = expanded_path;
            LOG_INFO("EngineConfiguration expanded source pack: " + this->asset_path);
            res_path.base_path = this->asset_path;
            res_path.source = ResourceSource::AssetPack;
        } else {
            LOG_ERR("EngineConfiguration specified asset source path is invalid: " + expanded_path);
            std::exit(1);
        }
        this->loader = ResourceLoader::GetResourceLoader(res_path);
        this->ReloadConfiguration();
    } else {
        LOG_ERR("EngineConfiguration specified asset source path is invalid: " + source_path);
        std::exit(1);
    }
}

std::string EngineConfiguration::GetExpandedPath(const std::string & unexpanded_path) {
#ifdef _WIN32
    char * expanded_path = _fullpath(NULL, unexpanded_path.c_str(), _MAX_PATH);
#else
    char * expanded_path = realpath(unexpanded_path.c_str(), NULL);
#endif
    if (expanded_path != nullptr) {
        std::string n_expanded_path{expanded_path};
        delete expanded_path;
        return n_expanded_path;
    } else {
        return std::string{};
    }
}

bool EngineConfiguration::ReloadConfiguration() {
    LOG_INFO("EngineConfiguration::ReloadConfiguration reading configuration from " CB_CONFIG_FILE);
    try {
        auto config_res = this->loader->OpenTextResource(CB_CONFIG_FILE);
        auto config = Toml::TomlParser{config_res};
        if (config.IsReady()) {
            this->valid = false;

            // debug settings
            this->debug.draw_gui_rects = config.GetTableBool("debug.draw_gui_rects", this->debug.draw_gui_rects);
            this->debug.draw_info_pane = config.GetTableBool("debug.draw_info_pane", this->debug.draw_info_pane);
            this->debug.draw_map_regions = config.GetTableBool("debug.draw_map_regions", this->debug.draw_map_regions);
            this->debug.draw_sprite_rects = config.GetTableBool("debug.draw_sprite_rects", this->debug.draw_sprite_rects);

            // input
            this->input.controller = config.GetTableBool("input.controller", this->input.controller);
            this->input.keyboard = config.GetTableBool("input.keyboard", this->input.keyboard);
            this->input.mouse = config.GetTableBool("input.mouse", this->input.mouse);

            // render seettings
            this->rendering.scale = config.GetTableFloat("rendering.scale", this->rendering.scale);

            // window settings
            this->window.full_screen = config.GetTableBool("window.full_screen", this->window.full_screen);
            this->window.height = config.GetTableInt("window.height", this->window.height);
            this->window.icon_path = config.GetTableString("window.icon", this->window.icon_path);
            this->window.title = config.GetTableString("window.title", this->window.title);
            this->window.width = config.GetTableInt("window.width", this->window.width);

            // fonts
            config.IterateTableArray("font", [this](const Toml::TomlParser & table) {
                CB_NamedFont named_font{
                    table.GetTableString("name"),
                    table.GetTableString("file"),
                    table.GetTableInt("size")
                };
                this->configured_fonts.push_back(named_font);
            });

            this->Validate();
        }
    } catch (cpptoml::parse_exception & e) {
        LOG_ERR("EngineConfiguration::ReloadConfiguration TOML parsing error " + std::string(e.what()));
    }

    return this->valid;
}

bool EngineConfiguration::Validate() {
    bool b_valid = true;

    if (this->window.width < 100 || this->window.height < 100) {
        b_valid = false;
    }

    if (!(this->input.controller || this->input.keyboard || this->input.mouse)) {
        b_valid = false;
    }

    // TODO: validate more settings ...

    this->valid = b_valid;
    return b_valid;
}
}