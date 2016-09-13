#include <critterbits.h>

namespace Critterbits {
namespace Toml {

TomlParser::TomlParser(const std::string & toml_file) {
    try {
        this->table = cpptoml::parse_file(toml_file);
        this->state = CBE_TOML_READY;
    } catch (cpptoml::parse_exception & e) {
        LOG_ERR("TomlParser::TomlParser TOML parsing error " + std::string(e.what()));
        this->state = CBE_TOML_ERR;
        this->parse_error = e.what();
    }
}

bool TomlParser::GetTableBool(const std::string & key, bool default_value) {
    return this->table->get_qualified_as<bool>(key).value_or(default_value);
}

float TomlParser::GetTableFloat(const std::string & key, float default_value) {
    return (float)this->table->get_qualified_as<double>(key).value_or((double)default_value);
}

int TomlParser::GetTableInt(const std::string & key, int default_value) {
    return (int)this->table->get_qualified_as<int64_t>(key).value_or((int64_t)default_value);
}

std::string TomlParser::GetTableString(const std::string & key, const std::string & default_value) {
    return this->table->get_qualified_as<std::string>(key).value_or(default_value);
}

}
}