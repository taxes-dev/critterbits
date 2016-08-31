#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits.h>
#include <map>
#include <stdlib.h>
#include <string>
#include <yaml.h>

#include <cblogging.h>
#include <cbyaml.h>

namespace Critterbits {

void YamlParser::Parse(void * context, const YamlParserCollection & parsers, const std::string & yaml_content) const {
    yaml_parser_t parser;
    yaml_token_t token;

    yaml_parser_initialize(&parser);
    unsigned char * yaml_content_u = new unsigned char[yaml_content.size() + 1];
    strncpy((char *)yaml_content_u, yaml_content.c_str(), yaml_content.length());
    yaml_parser_set_input_string(&parser, yaml_content_u, yaml_content.length());

    const YamlValueParser * current_parser = nullptr;
    YamlTokenType last_token = CBE_YAML_OTHER_TOKEN;

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
                // LOG_INFO("YamlParser::Parse key token");
                last_token = CBE_YAML_KEY_TOKEN;
                break;
            case YAML_VALUE_TOKEN:
                // LOG_INFO("YamlParser::Parse value token");
                last_token = CBE_YAML_VAL_TOKEN;
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
                // LOG_INFO("YamlParser::Parse scalar " + std::string((char *)token.data.scalar.value));
                if (last_token == CBE_YAML_KEY_TOKEN) {
                    auto search = parsers.find(std::string((char *)token.data.scalar.value));
                    if (search != parsers.end()) {
                        current_parser = &search->second;
                    } else {
                        LOG_ERR("YamlParser::Parse no parser for key " + std::string((char *)token.data.scalar.value));
                    }
                } else if (last_token == CBE_YAML_VAL_TOKEN) {
                    if (current_parser != nullptr) {
                        (*current_parser)(context, (const char *)token.data.scalar.value, token.data.scalar.length);
                        current_parser = nullptr;
                    }
                } else {
                    LOG_ERR("YamlParser::Parse unexpected scalar " + std::string((char *)token.data.scalar.value));
                }
                last_token = CBE_YAML_OTHER_TOKEN;
                break;
            default:
                // LOG_INFO("(other token)");
                last_token = CBE_YAML_OTHER_TOKEN;
                break;
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    delete yaml_content_u;
}

bool YamlParser::ToBool(const char * value) {
    if (value == NULL) {
        return false;
    }
    if (strcmp("true", value) == 0 || strcmp("on", value) == 0 || strcmp("yes", value) == 0) {
        return true;
    }
    return false;
}

float YamlParser::ToFloat(const char * value) {
    if (value == NULL) {
        return 0.0f;
    }
    float converted = strtof(value, NULL);
    if (errno == ERANGE) {
        LOG_ERR("YamlParser::ToFloat invalid value");
        return 0.0f;
    }
    return converted;
}

int YamlParser::ToInt(const char * value) {
    if (value == NULL) {
        return 0;
    }
    long int converted = strtol(value, NULL, 10);
    if (errno == ERANGE) {
        LOG_ERR("YamlParser::ToFloat invalid value");
        return 0;
    }
    return (int)converted;
}
}