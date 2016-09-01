#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits.h>
#include <list>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <yaml.h>

#include <cblogging.h>
#include <cbyaml.h>

namespace Critterbits {

namespace {
typedef enum { CBE_YAML_KEY_TOKEN, CBE_YAML_VAL_TOKEN, CBE_YAML_OTHER_TOKEN } YamlTokenType;
typedef enum { CBE_YAML_BLOCK_ENTRY, CBE_YAML_BLOCK_SEQUENCE, CBE_YAML_BLOCK_MAPPING } YamlBlockType;

std::string implode_map_names(std::list<std::string> & map_names, std::string & tail_name) {
    if (map_names.size() == 0) {
        return tail_name;
    }
    std::stringstream s;
    std::copy(map_names.begin(), map_names.end(), std::ostream_iterator<std::string>(s, "."));
    return s.str() + tail_name;
}
}

void YamlParser::Parse(void * context, const std::string & yaml_content) const {
    yaml_parser_t parser;
    yaml_token_t token;

    // set up parser to read from yaml_content
    yaml_parser_initialize(&parser);
    unsigned char * yaml_content_u = new unsigned char[yaml_content.size() + 1];
    strncpy((char *)yaml_content_u, yaml_content.c_str(), yaml_content.length());
    yaml_parser_set_input_string(&parser, yaml_content_u, yaml_content.length());

    // tracking variables
    const YamlSequenceParser * current_seq_parser = nullptr;
    const YamlValueParser * current_value_parser = nullptr;

    YamlTokenType last_token = CBE_YAML_OTHER_TOKEN;
    std::list<YamlBlockType> block_stack;
    std::list<std::string> seq_values;
    std::list<std::string> map_names;
    std::string scalar;
    std::string key_name;

    // loop through parser until end
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
            case YAML_BLOCK_SEQUENCE_START_TOKEN:
                // LOG_INFO("YamlParser::Parse Start Block (Sequence)");
                seq_values.clear();
                block_stack.push_back(CBE_YAML_BLOCK_SEQUENCE);
                break;
            case YAML_BLOCK_ENTRY_TOKEN:
                // LOG_INFO("YamlParser::Parse Start Block (Entry)");
                block_stack.push_back(CBE_YAML_BLOCK_ENTRY);
                last_token = CBE_YAML_VAL_TOKEN; // slight hack, YAML_SCALAR_TOKEN currently resets after each entry
                break;
            case YAML_BLOCK_END_TOKEN:
                // LOG_INFO("YamlParser::Parse End block");
                if (block_stack.size() > 0) {
                    if (block_stack.back() == CBE_YAML_BLOCK_SEQUENCE) {
                        if (current_seq_parser != nullptr) {
                            (*current_seq_parser)(context, seq_values);
                            current_seq_parser = nullptr;
                        } else {
                            LOG_ERR("YamlParser::Parse reached end of sequence with no assigned parser");
                        }
                    } else if (block_stack.back() == CBE_YAML_BLOCK_MAPPING && map_names.size() > 0) {
                        map_names.pop_back();
                    }
                    block_stack.pop_back();
                } else {
                    LOG_ERR("YamlParser::Parse reached block end without block start");
                }
                break;
            case YAML_BLOCK_MAPPING_START_TOKEN:
                // LOG_INFO("YamlParser::Parse [Block mapping]");
                block_stack.push_back(CBE_YAML_BLOCK_MAPPING);
                if (!scalar.empty()) {
                    map_names.push_back(scalar);
                }
                break;
            /* Flow */
            case YAML_FLOW_MAPPING_START_TOKEN:
                // LOG_INFO("YamlParser::Parse [flow mapping start]");
                if (!scalar.empty()) {
                    map_names.push_back(scalar);
                }
                break;
            case YAML_FLOW_MAPPING_END_TOKEN:
                // LOG_INFO("YamlParser::Parse [flow mapping end]");
                if (map_names.size() > 0) {
                    map_names.pop_back();
                }
                break;
            /* Data */
            case YAML_SCALAR_TOKEN:
                scalar = std::string((char *)token.data.scalar.value, token.data.scalar.length);
                if (last_token == CBE_YAML_KEY_TOKEN) {
                    key_name = implode_map_names(map_names, scalar);
                    // LOG_INFO("YamlParser::Parse key name " + key_name);

                    // look in both parser collections for the key name
                    auto search = this->value_parsers.find(key_name);
                    if (search != this->value_parsers.end()) {
                        current_value_parser = &search->second;
                        scalar = "";
                    } else {
                        auto search_2 = this->sequence_parsers.find(key_name);
                        if (search_2 != this->sequence_parsers.end()) {
                            current_seq_parser = &search_2->second;
                            scalar = "";
                        }
                    }
                    // if we fall through to here, scalar value is left intact in case it's a block name
                } else if (last_token == CBE_YAML_VAL_TOKEN) {
                    // LOG_INFO("YamlParser::Parse scalar value " + scalar);
                    // handle block value
                    if (current_value_parser != nullptr) {
                        (*current_value_parser)(context, scalar);
                        current_value_parser = nullptr;
                    } else if (current_seq_parser != nullptr) {
                        seq_values.push_back(scalar);
                    } else {
                        LOG_ERR("YamlParser::Parse unexpected value token " + scalar);
                    }
                    scalar = "";
                } else {
                    LOG_ERR("YamlParser::Parse unexpected scalar " + scalar);
                    scalar = "";
                }
                last_token = CBE_YAML_OTHER_TOKEN;
                break;
            default:
                // LOG_INFO("(other token) " + std::to_string(token.type));
                last_token = CBE_YAML_OTHER_TOKEN;
                break;
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    delete yaml_content_u;
}

bool YamlParser::ToBool(const std::string & value) {
    return value == "true" || value == "on" || value == "yes" || value == "1";
}

float YamlParser::ToFloat(const std::string & value) {
    try {
        return std::stof(value);
    } catch (std::exception &) {
        LOG_ERR("YamlParser::ToFloat invalid value");
        return 0.;
    }
}

int YamlParser::ToInt(const std::string & value) {
    try {
        return std::stoi(value);
    } catch (std::exception &) {
        LOG_ERR("YamlParser::ToInt invalid value");
        return 0;
    }
}
}