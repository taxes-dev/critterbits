#pragma once
#ifndef CBSCRIPTSUPPORT_HPP
#define CBSCRIPTSUPPORT_HPP
// Note: This file is only used internally by the scripting module, don't include in critterbits.h

#include <memory>
#include <string>

#include <cbcoord.hpp>
#include <cbentity.hpp>
#include <duktape/duktape.h>

#define CB_SCRIPT_ENTITY_STASH_ARRAY "entities"
#define CB_SCRIPT_HIDDEN_DESTROYED                                                                                     \
    "\xff"                                                                                                             \
    "destroyed"
#define CB_SCRIPT_HIDDEN_ENTITYID                                                                                      \
    "\xff"                                                                                                             \
    "entity_id"

// It's easy to leave stuff lying around on the duktape stack unintentionally. Use these as function guards.
#ifdef NDEBUG
#define CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(ctx)
#define CB_SCRIPT_ASSERT_STACK_CLEAN_END(ctx)
#define CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(ctx)
#define CB_SCRIPT_ASSERT_STACK_RETURN1_END(ctx)
#else
#define CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(ctx) \
  duk_idx_t _stack_clean_begin = duk_get_top(ctx);
#define CB_SCRIPT_ASSERT_STACK_CLEAN_END(ctx) \
  duk_idx_t _stack_clean_end = duk_get_top(ctx); \
  assert(_stack_clean_begin == _stack_clean_end);
#define CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(ctx) \
  duk_idx_t _stack_return1_begin = duk_get_top(ctx);
#define CB_SCRIPT_ASSERT_STACK_RETURN1_END(ctx) \
  duk_idx_t _stack_return1_end = duk_get_top(ctx); \
  assert(_stack_return1_begin + 1 == _stack_return1_end);
#endif 

namespace Critterbits {
namespace Scripting {

inline int GetPropertyBool(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    duk_bool_t value = duk_get_boolean(context, -1);
    duk_pop(context);
    return value != 0;
}

inline entity_id_t GetPropertyEntityId(duk_context * context, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, CB_SCRIPT_HIDDEN_ENTITYID);
    entity_id_t entity_id = duk_get_uint(context, -1);
    duk_pop(context);
    return entity_id;
}

inline int GetPropertyFloat(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    float value = (float)duk_get_number(context, -1);
    duk_pop(context);
    return value;
}

inline int GetPropertyInt(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    int value = duk_get_int(context, -1);
    duk_pop(context);
    return value;
}

inline CB_Rect GetPropertyRect(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    CB_Rect value;
    if (duk_is_object(context, -1)) {
        value.x = GetPropertyInt(context, "x");
        value.y = GetPropertyInt(context, "y");
        value.w = GetPropertyInt(context, "w");
        value.h = GetPropertyInt(context, "h");
    }
    duk_pop(context);
    return value;
}

inline std::string GetPropertyString(duk_context * context, const char * property_name, int stack_index = -1) {
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

inline void PushPropertyEntityId(duk_context * context, entity_id_t value) {
    duk_push_uint(context, value);
    duk_put_prop_string(context, -2, CB_SCRIPT_HIDDEN_ENTITYID);
}

inline void PushPropertyFloat(duk_context * context, const char * property_name, float value) {
    duk_push_number(context, value);
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyInt(duk_context * context, const char * property_name, int value) {
    duk_push_int(context, value);
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyRect(duk_context * context, const char * property_name, const CB_Rect & value) {
    duk_push_object(context);
    PushPropertyInt(context, "x", value.x);
    PushPropertyInt(context, "y", value.y);
    PushPropertyInt(context, "w", value.w);
    PushPropertyInt(context, "h", value.h);
    duk_put_prop_string(context, -2, property_name);
}

inline void PushPropertyString(duk_context * context, const char * property_name, const std::string & value) {
    duk_push_string(context, value.c_str());
    duk_put_prop_string(context, -2, property_name);
}

void ClearEntitiesInContext(duk_context *);

void CreateEntityInContext(duk_context *, std::shared_ptr<Entity>);

void RetrieveEntityFromContext(duk_context *, std::shared_ptr<Entity>);

void RetrieveEntityFromContextAt(duk_context *, std::shared_ptr<Entity>, int = -1);
}
}
#endif