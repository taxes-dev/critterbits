#include <SDL.h>
#include <fstream>
#include <string>

#include "cbengine.h"
#include "cblogging.h"
#include "cbyaml.h"

#ifdef _WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

namespace Critterbits {

/*
 * Support functions for EngineConfiguration::ParseYaml(const std::string&)
 */
static void window_height_parser(void * context, const char * value, const size_t size) {
    static_cast<EngineConfiguration *>(context)->window_height = YamlParser::to_int(value);
}

static void window_width_parser(void * context, const char * value, const size_t size) {
    static_cast<EngineConfiguration *>(context)->window_width = YamlParser::to_int(value);
}

static YamlParserCollection config_parsers = {{"window_height", window_height_parser},
                                              {"window_width", window_width_parser}};
/*
 * End support functions
 */

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

    LOG_INFO("EngineConfiguration unexpanded source path: " + unexpanded_path);

    char * expanded_path = realpath(unexpanded_path.c_str(), NULL);
    if (expanded_path != NULL) {
        this->asset_path = std::string(expanded_path) + PATH_SEP;
        LOG_INFO("EngineConfiguration expanded source path: " + this->asset_path);
        free(expanded_path);
        this->ReloadConfiguration();
    }
}

bool EngineConfiguration::ReloadConfiguration() {
    std::fstream config_file;

    this->valid = false;

    LOG_INFO("EngineConfiguration::ReloadConfiguration reading configuration from " + this->asset_path +
             CB_CONFIG_YAML);
    config_file.open(this->asset_path + CB_CONFIG_YAML, std::fstream::in);
    if (config_file.good()) {
        // read entire contents into a string (config file should not be very big, so this is fine)
        std::string config_content((std::istreambuf_iterator<char>(config_file)), (std::istreambuf_iterator<char>()));
        config_file.close();

        // parse YAML into configuration
        YamlParser parser;
        parser.Parse(this, config_parsers, config_content);

        this->valid = true;
    }

    return this->valid;
}
}