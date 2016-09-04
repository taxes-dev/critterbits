#include <cstring>

#include <critterbits.h>

#include <duktape/duktape.h>

#define CB_SCRIPT_GLOBAL_UPDATE "update"

#define CB_SCRIPT_ENTITY_THIS "entity_this"

namespace Critterbits {
/*
* Support functions for modifying the duktape context
*/
namespace {
inline int get_int_property(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    int value = duk_get_int(context, -1);
    duk_pop(context);
    return value;
}

inline void push_property_int(duk_context * context, const char * property_name, int value) {
    duk_push_int(context, value);
    duk_put_prop_string(context, -2,
                        property_name); // -2 skips over int just pushed and assumes target object is right behind it
}

inline void push_property_string(duk_context * context, const char * property_name, std::string & value) {
    duk_push_string(context, value.c_str());
    duk_put_prop_string(context, -2,
                        property_name); // -2 skips over string just pushed and assumes target object is right behind it
}
}
/*
 * End support functions
 */

void Script::DiscoverGlobals() {
    LOG_INFO("Script::DiscoverGlobals starting for " + this->script_name);
    if (this->context == nullptr) {
        return;
    }

    // discover which functions are available that the engine is interested in calling

    // first get a list of all the properties on the global object
    duk_get_global_string(this->context, "Object");
    duk_get_prop_string(this->context, -1, "getOwnPropertyNames");
    duk_push_global_object(this->context);
    duk_call(this->context, 1);

    // now iterate the list and look for ones we're interested in
    int num_globals = duk_get_length(this->context, -1);
    for (int i = 0; i < num_globals; i++) {
        if (duk_get_prop_index(this->context, -1, i) != 0) {
            const char * prop_name = duk_safe_to_string(this->context, -1);
            if (strcmp(prop_name, CB_SCRIPT_GLOBAL_UPDATE) == 0) {
                this->global_update = true;
            }
        } else {
            LOG_ERR("Script::DiscoverGlobals error retrieving property at index " + std::to_string(i));
        }
        duk_pop(this->context);
    }

    duk_pop_2(this->context);
}

void Script::CreateEntityInContext(std::shared_ptr<Entity> entity, const char * stash_name) {
    duk_push_global_stash(this->context); // will save to the global stash when we're done

    duk_push_object(this->context); // root

    duk_push_object(this->context); // dim
    push_property_int(this->context, "x", entity->dim.x);
    push_property_int(this->context, "y", entity->dim.y);
    push_property_int(this->context, "w", entity->dim.w);
    push_property_int(this->context, "h", entity->dim.h);
    duk_put_prop_string(this->context, -2, "dim");

    push_property_string(this->context, "tag", entity->tag);

    // save to the stash so we can grab it after method call
    duk_put_prop_string(this->context, -2, stash_name);

    // now put the entity back on the top of the stack
    duk_get_prop_string(this->context, -1, stash_name);
    duk_swap_top(this->context, -2);
    duk_pop(this->context);
}

void Script::RetrieveEntityFromContext(std::shared_ptr<Entity> entity, const char * stash_name) {
    duk_push_global_stash(this->context);
    if (duk_get_prop_string(this->context, -1, stash_name) != 0) {
        // only mutable properties are retrieved
        duk_get_prop_string(this->context, -1, "dim");
        entity->dim.x = get_int_property(this->context, "x");
        entity->dim.y = get_int_property(this->context, "y");
        entity->dim.w = get_int_property(this->context, "w");
        entity->dim.h = get_int_property(this->context, "h");
        duk_pop(this->context); // dim
        duk_pop(this->context); // "stash_name"
    } else {
        LOG_ERR("Script::RetrieveEntityFromContext unable to retrieve entity from stash");
    }
    duk_pop(this->context); // stash
}

void Script::CallUpdate(std::shared_ptr<Entity> entity, float delta_time) {
    if (this->global_update) {
        // setup call to global update script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_UPDATE);
        CreateEntityInContext(entity, CB_SCRIPT_ENTITY_THIS);
        duk_push_number(this->context, delta_time);
        duk_call_method(this->context, 1);

        // clean up and pull any changes to the entity
        duk_pop_2(this->context);
        RetrieveEntityFromContext(entity, CB_SCRIPT_ENTITY_THIS);
    }
}
}