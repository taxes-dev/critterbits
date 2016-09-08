#pragma once
#ifndef CBSCRIPTSUPPORT_HPP
#define CBSCRIPTSUPPORT_HPP
// Note: This file is only used internally by the scripting module, don't include in critterbits.h

#include <string>

#include <cbentity.h>
#include <duktape/duktape.h>

namespace Critterbits {
namespace Scripting {
inline int GetBoolProperty(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    duk_bool_t value = duk_get_boolean(context, -1);
    duk_pop(context);
    return value != 0;
}

inline int GetFloatProperty(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    float value = (float)duk_get_number(context, -1);
    duk_pop(context);
    return value;
}

inline int GetIntProperty(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    int value = duk_get_int(context, -1);
    duk_pop(context);
    return value;
}

inline std::string GetStringProperty(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    const char * value = duk_get_string(context, -1);
    duk_pop(context);
    if (value == nullptr) {
        return "";
    }
    return std::string(value);
}

inline void PushPropertyBool(duk_context * context, const char * property_name, bool value) {
    duk_push_boolean(context, value ? 1 : 0);
    // -2 skips over scalar just pushed and assumes target object is right behind it
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyFloat(duk_context * context, const char * property_name, float value) {
    duk_push_number(context, value);
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyInt(duk_context * context, const char * property_name, int value) {
    duk_push_int(context, value);
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyString(duk_context * context, const char * property_name, const std::string & value) {
    duk_push_string(context, value.c_str());
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyEntityId(duk_context * context, entity_id_t value) {
    duk_push_uint(context, value);
    duk_put_prop_string(context, -2, CB_SCRIPT_HIDDEN_ENTITYID);
}
}
}
#endif