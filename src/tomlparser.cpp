#include <algorithm>

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

bool TomlParser::GetTableBool(const std::string & key, bool default_value) const {
    return this->table->get_qualified_as<bool>(key).value_or(default_value);
}

void TomlParser::GetTableFlexRect(const std::string & key, FlexRect * flex_rect) const {
    if (flex_rect != nullptr) {
        auto table2 = this->table->get_table_qualified(key);
        if (table2) {
            // top, left
            auto val = table2->get_as<int64_t>("top");
            flex_rect->top = val ? std::abs(static_cast<int>(*val)) : FlexRect::FLEX;
            val = table2->get_as<int64_t>("left");
            flex_rect->left = val ? std::abs(static_cast<int>(*val)) : FlexRect::FLEX;

            // bottom, right
            val = table2->get_as<int64_t>("bottom");
            flex_rect->bottom = val ? std::abs(static_cast<int>(*val)) : FlexRect::FLEX;
            val = table2->get_as<int64_t>("right");
            flex_rect->right = val ? std::abs(static_cast<int>(*val)) : FlexRect::FLEX;

            // width, height
            val = table2->get_as<int64_t>("width");
            flex_rect->width = val ? std::abs(static_cast<int>(*val)) : FlexRect::FLEX;
            val = table2->get_as<int64_t>("height");
            flex_rect->height = val ? std::abs(static_cast<int>(*val)) : FlexRect::FLEX;

        }
    }
}

float TomlParser::GetTableFloat(const std::string & key, float default_value) const {
    return static_cast<float>(this->table->get_qualified_as<double>(key).value_or(static_cast<double>(default_value)));
}

int TomlParser::GetTableInt(const std::string & key, int default_value) const {
    return static_cast<int>(this->table->get_qualified_as<int64_t>(key).value_or(static_cast<int64_t>(default_value)));
}

CB_Point TomlParser::GetTablePoint(const std::string & key, const CB_Point & default_value) const {
    auto table2 = this->table->get_table_qualified(key);
    int64_t x = default_value.x, y = default_value.y;
    if (table2) {
        x = table2->get_as<int64_t>("x").value_or(default_value.x);
        y = table2->get_as<int64_t>("y").value_or(default_value.y);
    }
    return CB_Point{static_cast<int>(x), static_cast<int>(y)};
}

CB_Rect TomlParser::GetTableRect(const std::string & key, const CB_Rect & default_value) const {
    auto table2 = this->table->get_table_qualified(key);
    int64_t x = default_value.x, y = default_value.y, w = default_value.w, h = default_value.h;
    if (table2) {
        x = table2->get_as<int64_t>("x").value_or(default_value.x);
        y = table2->get_as<int64_t>("y").value_or(default_value.y);
        w = table2->get_as<int64_t>("w").value_or(default_value.w);
        h = table2->get_as<int64_t>("h").value_or(default_value.h);
    }
    return CB_Rect{
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(w),
        static_cast<int>(h)
    };
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