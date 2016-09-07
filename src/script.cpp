#include <cstring>

#include <critterbits.h>

#include <duktape/duktape.h>

namespace Critterbits {
/*
* Support functions for modifying the duktape context
*/
namespace {
inline int get_bool_property(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    duk_bool_t value = duk_get_boolean(context, -1);
    duk_pop(context);
    return value != 0;
}

inline int get_float_property(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    float value = (float)duk_get_number(context, -1);
    duk_pop(context);
    return value;
}

inline int get_int_property(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    int value = duk_get_int(context, -1);
    duk_pop(context);
    return value;
}

inline std::string get_string_property(duk_context * context, const char * property_name, int stack_index = -1) {
    duk_get_prop_string(context, stack_index, property_name);
    const char * value = duk_get_string(context, -1);
    duk_pop(context);
    if (value == nullptr) {
        return "";
    }
    return std::string(value);
}

inline void push_property_bool(duk_context * context, const char * property_name, bool value) {
    duk_push_boolean(context, value ? 1 : 0);
    // -2 skips over scalar just pushed and assumes target object is right behind it
    duk_put_prop_string(context, -2, property_name);
}

inline void push_property_float(duk_context * context, const char * property_name, float value) {
    duk_push_number(context, value);
    duk_put_prop_string(context, -2, property_name);
}

inline void push_property_int(duk_context * context, const char * property_name, int value) {
    duk_push_int(context, value);
    duk_put_prop_string(context, -2, property_name);
}

inline void push_property_string(duk_context * context, const char * property_name, const std::string & value) {
    duk_push_string(context, value.c_str());
    duk_put_prop_string(context, -2, property_name);
}

inline void push_property_entity_id(duk_context * context, entity_id_t value) {
    duk_push_uint(context, value);
    duk_put_prop_string(context, -2, CB_SCRIPT_HIDDEN_ENTITYID);
}
/*
* Functions callable from JavaScript code
*/
duk_ret_t mark_entity_destroyed(duk_context * context) {
    duk_push_this(context);
    push_property_bool(context, CB_SCRIPT_HIDDEN_DESTROYED, true);
    duk_pop(context);
    return 0;
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
            } else if (strcmp(prop_name, CB_SCRIPT_GLOBAL_START) == 0) {
                this->global_start = true;
            } else if (strcmp(prop_name, CB_SCRIPT_GLOBAL_ONCOLLISION) == 0) {
                this->global_oncollision = true;
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

    push_property_entity_id(this->context, entity->entity_id);
    push_property_string(this->context, "tag", entity->tag);
    push_property_float(this->context, "time_scale", entity->time_scale);

    duk_push_c_function(this->context, mark_entity_destroyed, 0);
    duk_put_prop_string(this->context, -2, "destroy");

    if (entity->GetEntityType() == CBE_SPRITE) {
        this->ExtendEntityWithSprite(std::dynamic_pointer_cast<Sprite>(entity));
    }

    // save to the stash so we can grab it after method call
    duk_put_prop_string(this->context, -2, stash_name);

    // now put the entity back on the top of the stack
    duk_get_prop_string(this->context, -1, stash_name);
    duk_swap_top(this->context, -2);
    duk_pop(this->context);
}

void Script::ExtendEntityWithSprite(std::shared_ptr<Sprite> sprite) {
    push_property_string(this->context, "entity_type", "sprite");
    push_property_float(this->context, "sprite_scale", sprite->sprite_scale);
    push_property_int(this->context, "tile_height", sprite->tile_height);
    push_property_int(this->context, "tile_width", sprite->tile_width);
    push_property_int(this->context, "tile_offset_x", sprite->tile_offset_x);
    push_property_int(this->context, "tile_offset_y", sprite->tile_offset_y);
    push_property_bool(this->context, "flip_x", sprite->flip_x);
    push_property_bool(this->context, "flip_y", sprite->flip_y);

    duk_push_object(this->context); // frame
    push_property_int(this->context, "current", sprite->GetFrame());
    push_property_int(this->context, "count", sprite->GetFrameCount());
    duk_put_prop_string(this->context, -2, "frame");
}

void Script::RetrieveEntityFromContext(std::shared_ptr<Entity> entity, const char * stash_name) {
    duk_push_global_stash(this->context);
    if (duk_get_prop_string(this->context, -1, stash_name) != 0) {
        // only mutable properties are retrieved
        duk_get_prop_string(this->context, -1, "dim");
        int new_x = get_int_property(this->context, "x");
        int new_y = get_int_property(this->context, "y");
        entity->dim.w = get_int_property(this->context, "w");
        entity->dim.h = get_int_property(this->context, "h");
        duk_pop(this->context); // dim
        entity->time_scale = get_float_property(this->context, "time_scale");

        // set position (may result in collisions)
        entity->SetPosition(new_x, new_y);

        // special case: "destroyed flag"
        bool destroyed = get_bool_property(this->context, CB_SCRIPT_HIDDEN_DESTROYED);
        if (destroyed) {
            entity->MarkDestroy();
        }

        if (entity->GetEntityType() == CBE_SPRITE) {
            this->RetrieveSpriteFromContext(std::dynamic_pointer_cast<Sprite>(entity));
        }

        duk_pop(this->context); // "stash_name"
    } else {
        LOG_ERR("Script::RetrieveEntityFromContext unable to retrieve entity from stash");
    }
    duk_pop(this->context); // stash
}

void Script::RetrieveSpriteFromContext(std::shared_ptr<Sprite> sprite) {
    sprite->sprite_scale = get_float_property(this->context, "sprite_scale");
    sprite->tile_height = get_int_property(this->context, "tile_height");
    sprite->tile_width = get_int_property(this->context, "tile_width");
    sprite->tile_offset_x = get_int_property(this->context, "tile_offset_x");
    sprite->tile_offset_y = get_int_property(this->context, "tile_offset_y");
    sprite->flip_x = get_bool_property(this->context, "flip_x");
    sprite->flip_y = get_bool_property(this->context, "flip_y");
    duk_get_prop_string(this->context, -1, "frame");
    int current_frame = get_int_property(this->context, "current");
    sprite->SetFrame(current_frame);
    duk_pop(this->context); // frame
}

void Script::CallOnCollision(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> other_entity) {
    if (this->global_oncollision) {
        std::string other_entity_name{CB_SCRIPT_ENTITY_NAME(other_entity)};

        // setup call to global oncollision script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_ONCOLLISION);
        CreateEntityInContext(entity, CB_SCRIPT_ENTITY_THIS);
        CreateEntityInContext(other_entity, other_entity_name.c_str());
        duk_call_method(this->context, 1);

        // clean up and pull any changes to the entities
        duk_pop_2(this->context);
        RetrieveEntityFromContext(entity, CB_SCRIPT_ENTITY_THIS);
        RetrieveEntityFromContext(other_entity, other_entity_name.c_str());
    }
}

void Script::CallStart(std::shared_ptr<Entity> entity) {
    if (this->global_start) {
        // setup call to global start script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_START);
        CreateEntityInContext(entity, CB_SCRIPT_ENTITY_THIS);
        duk_call_method(this->context, 0);

        // clean up and pull any changes to the entity
        duk_pop_2(this->context);
        RetrieveEntityFromContext(entity, CB_SCRIPT_ENTITY_THIS);
    }
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