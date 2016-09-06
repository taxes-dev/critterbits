#pragma once
#ifndef CBYAML_H
#define CBYAML_H

#include <functional>
#include <list>
#include <map>
#include <string>

// define this to get really noisy logging from the YAML parser
//#define CB_YAML_LOGGING

#include "cblogging.h"
#ifdef CB_YAML_LOGGING
#define LOG_YAML(m) LOG_INFO(m)
#else
#define LOG_YAML(m)
#endif

namespace Critterbits {
typedef std::function<void(void *, std::string value)> YamlValueParser;
typedef std::function<void(void *, std::list<std::string> &)> YamlSequenceParser;
typedef std::map<std::string, YamlValueParser> YamlValueParserCollection;
typedef std::map<std::string, YamlSequenceParser> YamlSequenceParserCollection;

class YamlParser {
  public:
    YamlSequenceParserCollection sequence_parsers;
    YamlValueParserCollection value_parsers;

    YamlParser(){};
    void Parse(void *, const std::string &) const;
    static bool ToBool(const std::string &);
    static float ToFloat(const std::string &);
    static int ToInt(const std::string &);
};
}
#endif