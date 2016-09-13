#pragma once
#ifndef CBTOML_H
#define CBTOML_H

#include <memory>
#include <string>

#include <cpptoml/cpptoml.h>

namespace Critterbits {
namespace Toml {

typedef enum {CBE_TOML_NEW, CBE_TOML_READY, CBE_TOML_ERR} TomlParserState;

class TomlParser {
    public:
        TomlParser(const std::string &);

        std::string GetParserError() const {
            if (this->state == CBE_TOML_ERR) {
                return this->parse_error;
            } else {
                return std::string{};
            }
        };
        bool GetTableBool(const std::string &, bool = false);
        float GetTableFloat(const std::string &, float = 0.f);
        int GetTableInt(const std::string &, int = 0);
        std::string GetTableString(const std::string &, const std::string & = "");
        bool IsReady() const { return this->state == CBE_TOML_READY; };

    private:
        TomlParserState state{CBE_TOML_NEW};
        std::shared_ptr<cpptoml::table> table;
        std::string parse_error;
};

}
}
#endif