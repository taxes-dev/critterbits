#pragma once
#ifndef CBTOML_HPP
#define CBTOML_HPP

#include <functional>
#include <istream>
#include <memory>
#include <string>

#include <cpptoml/cpptoml.h>

#include "coord.hpp"

namespace Critterbits {
namespace Toml {

enum class TomlParserState { New, Ready, Error };

class TomlParser {
  public:
    TomlParser(std::shared_ptr<std::istream>);

    std::string GetParserError() const {
        if (this->state == TomlParserState::Error) {
            return this->parse_error;
        } else {
            return std::string{};
        }
    };
    bool GetTableBool(const std::string &, bool = false) const;
    CB_Color GetTableColor(const std::string &, CB_Color = {}) const;
    void GetTableFlexRect(const std::string &, FlexRect *) const;
    float GetTableFloat(const std::string &, float = 0.f) const;
    int GetTableInt(const std::string &, int = 0) const;
    CB_Point GetTablePoint(const std::string &, const CB_Point & = {}) const;
    CB_Rect GetTableRect(const std::string &, const CB_Rect & = {}) const;
    std::string GetTableString(const std::string &, const std::string & = "") const;
    bool IsReady() const { return this->state == TomlParserState::Ready; };
    void IterateTableArray(const std::string &, const std::function<void(const TomlParser &)> &) const;

  private:
    TomlParserState state;
    std::shared_ptr<cpptoml::table> table;
    std::string parse_error;

    TomlParser() : state(TomlParserState::New){};
    TomlParser(std::shared_ptr<cpptoml::table> _table) : state(TomlParserState::Ready), table(_table){};
};
}
}
#endif