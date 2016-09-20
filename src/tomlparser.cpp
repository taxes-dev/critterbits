#include <cb/critterbits.hpp>

namespace Critterbits {
namespace Toml {

TomlParser::TomlParser(std::shared_ptr<std::istream> stream) : TomlParser() {
    if (stream->good()) {
        try {
            cpptoml::parser parser{*stream.get()};
            this->table = parser.parse();
            this->state = TomlParserState::Ready;
        } catch (cpptoml::parse_exception & e) {
            LOG_ERR("TomlParser::TomlParser TOML parsing error " + std::string(e.what()));
            this->state = TomlParserState::Error;
            this->parse_error = e.what();            
        }
    } else {
        LOG_ERR("TomlParser::TomlParser cannot read from provided stream");
        this->state = TomlParserState::Error;
        this->parse_error = "bad stream";
    }
}

TomlParser::TomlParser(const std::string & toml_file) : TomlParser() {
    try {
        this->table = cpptoml::parse_file(toml_file);
        this->state = TomlParserState::Ready;
    } catch (cpptoml::parse_exception & e) {
        LOG_ERR("TomlParser::TomlParser TOML parsing error " + std::string(e.what()));
        this->state = TomlParserState::Error;
        this->parse_error = e.what();
    }
}

bool TomlParser::GetTableBool(const std::string & key, bool default_value) const {
    return this->table->get_qualified_as<bool>(key).value_or(default_value);
}

float TomlParser::GetTableFloat(const std::string & key, float default_value) const {
    return (float)this->table->get_qualified_as<double>(key).value_or((double)default_value);
}

int TomlParser::GetTableInt(const std::string & key, int default_value) const {
    return (int)this->table->get_qualified_as<int64_t>(key).value_or((int64_t)default_value);
}

CB_Point TomlParser::GetTablePoint(const std::string & key, CB_Point default_value) const {
    auto table2 = this->table->get_table(key);
    int64_t x = default_value.x, y = default_value.y;
    if (table2) {
        x = table2->get_as<int64_t>("x").value_or(default_value.x);
        y = table2->get_as<int64_t>("y").value_or(default_value.y);
    }
    return CB_Point{(int)x, (int)y};
}

std::string TomlParser::GetTableString(const std::string & key, const std::string & default_value) const {
    return this->table->get_qualified_as<std::string>(key).value_or(default_value);
}

void TomlParser::IterateTableArray(const std::string & table_name,
                                   const std::function<void(const TomlParser &)> & iterator) const {
    auto tarr = this->table->get_table_array(table_name);

    if (tarr) {
        for (const auto & arr_table : *tarr) {
            TomlParser subparser{arr_table};
            iterator(subparser);
        }
    }
}
}
}