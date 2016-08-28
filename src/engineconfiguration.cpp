#include <SDL.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <limits.h>
#include <map>
#include <stdlib.h>
#include <string>

#include "cbengine.h"
#include "cblogging.h"
#include "yaml.h"

#ifdef _WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

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
        this->ParseYaml(config_content);

        this->valid = true;
    }

    return this->valid;
}

/*
 * Support functions for EngineConfiguration::ParseYaml(const std::string&)
 */
typedef enum { E_CFG_KEY_TOKEN, E_CFG_VAL_TOKEN, E_CFG_OTHER_TOKEN } ConfigTokenType;
typedef std::function<void(EngineConfiguration *, yaml_token_t &)> ConfigTokenParser;

static void window_height_parser(EngineConfiguration * config, yaml_token_t & token) {
    if (token.data.scalar.style == YAML_PLAIN_SCALAR_STYLE) {
        long int h = strtol((const char *)token.data.scalar.value, NULL, 10);
        if (errno == ERANGE) {
            LOG_ERR("window_height_parser invalid value");
        } else {
            config->window_height = (int)h;
        }
    }
}

static void window_width_parser(EngineConfiguration * config, yaml_token_t & token) {
    if (token.data.scalar.style == YAML_PLAIN_SCALAR_STYLE) {
        long int w = strtol((const char *)token.data.scalar.value, NULL, 10);
        if (errno == ERANGE) {
            LOG_ERR("window_width_parser invalid value");
        } else {
            config->window_width = (int)w;
        }
    }
}
/*
 * End support functions
 */

void EngineConfiguration::ParseYaml(const std::string & yaml_content) {
    // TODO: make generic... will likely want to parse YAML elsewhere

    yaml_parser_t parser;
    yaml_token_t token;

    yaml_parser_initialize(&parser);
    unsigned char * yaml_content_u = new unsigned char[yaml_content.size() + 1];
    strncpy((char *)yaml_content_u, yaml_content.c_str(), yaml_content.length());
    yaml_parser_set_input_string(&parser, yaml_content_u, yaml_content.length());

    static std::map<std::string, ConfigTokenParser> parsers = {{"window_height", window_height_parser},
                                                               {"window_width", window_width_parser}};
    ConfigTokenParser * current_parser = nullptr;
    ConfigTokenType last_token = E_CFG_OTHER_TOKEN;

    while (true) {
        if (!yaml_parser_scan(&parser, &token)) {
            break;
        }
        if (token.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        switch (token.type) {
            /* Token types (read before actual token) */
            case YAML_KEY_TOKEN:
                LOG_INFO("(Key token)   ");
                last_token = E_CFG_KEY_TOKEN;
                break;
            case YAML_VALUE_TOKEN:
                LOG_INFO("(Value token) ");
                last_token = E_CFG_VAL_TOKEN;
                break;
            /* Block delimeters */
            /*case YAML_BLOCK_SEQUENCE_START_TOKEN:
                LOG_INFO("Start Block (Sequence)");
                break;
            case YAML_BLOCK_ENTRY_TOKEN:
                LOG_INFO("Start Block (Entry)");
                break;
            case YAML_BLOCK_END_TOKEN:
                LOG_INFO("End block");
                break;*/
            /* Data */
            /*case YAML_BLOCK_MAPPING_START_TOKEN:
                LOG_INFO("[Block mapping]");
                break;*/
            case YAML_SCALAR_TOKEN:
                LOG_INFO("Scalar " + std::string((char *)token.data.scalar.value));
                if (last_token == E_CFG_KEY_TOKEN) {
                    auto search = parsers.find(std::string((char *)token.data.scalar.value));
                    if (search != parsers.end()) {
                        current_parser = &search->second;
                    }
                } else if (last_token == E_CFG_VAL_TOKEN) {
                    if (current_parser != nullptr) {
                        (*current_parser)(this, token);
                        current_parser = nullptr;
                    }
                } else {
                    LOG_ERR("EngineConfiguration::ParseYaml unexpected scalar");
                }
                last_token = E_CFG_OTHER_TOKEN;
                break;
            default:
                // LOG_INFO("(other token)");
                last_token = E_CFG_OTHER_TOKEN;
                break;
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    delete yaml_content_u;
}
}