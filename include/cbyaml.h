#pragma once
#ifndef CBYAML_H
#define CBYAML_H

#include <functional>
#include <map>
#include <string>

namespace Critterbits {
typedef enum { CBE_YAML_KEY_TOKEN, CBE_YAML_VAL_TOKEN, CBE_YAML_OTHER_TOKEN } YamlTokenType;
typedef std::function<void(void *, const char *, const size_t)> YamlValueParser;
typedef std::map<std::string, YamlValueParser> YamlParserCollection;

class YamlParser {
  public:
    YamlParser(){};
    void Parse(void *, const YamlParserCollection &, const std::string &) const;
    static bool ToBool(const char *);
    static float ToFloat(const char *);
    static int ToInt(const char *);
};
}
#endif