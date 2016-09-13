#pragma once
#ifndef CBTOML_H
#define CBTOML_H

#include <functional>
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
        bool GetTableBool(const std::string &, bool = false) const;
        float GetTableFloat(const std::string &, float = 0.f) const;
        int GetTableInt(const std::string &, int = 0) const;
        std::string GetTableString(const std::string &, const std::string & = "") const;
        bool IsReady() const { return this->state == CBE_TOML_READY; };
        void IterateTableArray(const std::string &, const std::function<void(const TomlParser &)> &) const;

    private:
        TomlParserState state;
        std::shared_ptr<cpptoml::table> table;
        std::string parse_error;

        TomlParser() : state(CBE_TOML_NEW) {};
        TomlParser(std::shared_ptr<cpptoml::table> _table) : state{CBE_TOML_READY}, table(_table) {};
};

}
}
#endif