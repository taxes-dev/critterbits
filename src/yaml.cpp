#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits.h>
#include <list>
#include <map>
#include <stdlib.h>
#include <string>
#include <yaml.h>

#include <cblogging.h>
#include <cbyaml.h>

namespace Critterbits {

typedef enum { CBE_YAML_KEY_TOKEN, CBE_YAML_VAL_TOKEN, CBE_YAML_OTHER_TOKEN } YamlTokenType;
typedef enum { CBE_YAML_BLOCK_ENTRY, CBE_YAML_BLOCK_SEQUENCE, CBE_YAML_BLOCK_MAPPING } YamlBlockType;

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
    std::string scalar;

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
                LOG_INFO("YamlParser::Parse key token");
                last_token = CBE_YAML_KEY_TOKEN;
                break;
            case YAML_VALUE_TOKEN:
                LOG_INFO("YamlParser::Parse value token");
                last_token = CBE_YAML_VAL_TOKEN;
                break;
            /* Block delimeters */
            case YAML_BLOCK_SEQUENCE_START_TOKEN:
                LOG_INFO("Start Block (Sequence)");
                seq_values.clear();
                block_stack.push_back(CBE_YAML_BLOCK_SEQUENCE);
                break;
            case YAML_BLOCK_ENTRY_TOKEN:
                LOG_INFO("Start Block (Entry)");
                block_stack.push_back(CBE_YAML_BLOCK_ENTRY);
                last_token = CBE_YAML_VAL_TOKEN; // slight hack, YAML_SCALAR_TOKEN currently resets after each entry
                break;
            case YAML_BLOCK_END_TOKEN:
                LOG_INFO("End block");
                if (block_stack.back() == CBE_YAML_BLOCK_SEQUENCE) {
                    if (current_seq_parser != nullptr) {
                        (*current_seq_parser)(context, seq_values);
                        current_seq_parser = nullptr;
                    } else {
                        LOG_ERR("YamlParser::Parse reached end of sequence with no assigned parser");
                    }
                }
                block_stack.pop_back();
                break;
            /* Data */
            case YAML_BLOCK_MAPPING_START_TOKEN:
                LOG_INFO("[Block mapping]");
                block_stack.push_back(CBE_YAML_BLOCK_MAPPING);
                break;
            case YAML_SCALAR_TOKEN:
                scalar = std::string((char *)token.data.scalar.value, token.data.scalar.length);
                LOG_INFO("YamlParser::Parse scalar " + std::string(scalar));
                if (last_token == CBE_YAML_KEY_TOKEN) {
                    // look in both parser collections for the key name
                    auto search = this->value_parsers.find(scalar);
                    if (search != this->value_parsers.end()) {
                        current_value_parser = &search->second;
                    } else {
                        auto search_2 = this->sequence_parsers.find(scalar);
                        if (search_2 != this->sequence_parsers.end()) {
                            current_seq_parser = &search_2->second;
                        } else {
                            LOG_ERR("YamlParser::Parse no parser for key " + scalar);
                        }
                    }
                } else if (last_token == CBE_YAML_VAL_TOKEN) {
                    // handle block value
                    if (current_value_parser != nullptr) {
                        (*current_value_parser)(context, scalar);
                        current_value_parser = nullptr;
                    } else if (current_seq_parser != nullptr) {
                        seq_values.push_back(scalar);
                    } else {
                        LOG_ERR("YamlParser::Parse unexpected value token " + scalar);
                    }
                } else {
                    LOG_ERR("YamlParser::Parse unexpected scalar " + scalar);
                }
                last_token = CBE_YAML_OTHER_TOKEN;
                break;
            default:
                LOG_INFO("(other token) " + std::to_string(token.type));
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