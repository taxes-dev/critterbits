#include <cstring>

#include <critterbits.h>
#include <scripting/cbscriptsupport.hpp>

namespace Critterbits {
namespace Scripting {
/*
* Functions callable from JavaScript code
*/
namespace {
duk_ret_t mark_entity_destroyed(duk_context * context) {
    duk_push_this(context);
    PushPropertyBool(context, CB_SCRIPT_HIDDEN_DESTROYED, true);
    duk_pop(context);
    return 0;
}
}
/*
 * End callable support functions
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
    PushPropertyRect(this->context, "dim", entity->dim);
    PushPropertyEntityId(this->context, entity->entity_id);
    PushPropertyString(this->context, "tag", entity->tag);
    PushPropertyFloat(this->context, "time_scale", entity->time_scale);

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
    PushPropertyString(this->context, "entity_type", "sprite");
    PushPropertyFloat(this->context, "sprite_scale", sprite->sprite_scale);
    PushPropertyInt(this->context, "tile_height", sprite->tile_height);
    PushPropertyInt(this->context, "tile_width", sprite->tile_width);
    PushPropertyInt(this->context, "tile_offset_x", sprite->tile_offset_x);
    PushPropertyInt(this->context, "tile_offset_y", sprite->tile_offset_y);
    PushPropertyBool(this->context, "flip_x", sprite->flip_x);
    PushPropertyBool(this->context, "flip_y", sprite->flip_y);

    duk_push_object(this->context); // frame
    PushPropertyInt(this->context, "current", sprite->GetFrame());
    PushPropertyInt(this->context, "count", sprite->GetFrameCount());
    duk_put_prop_string(this->context, -2, "frame");
}

void Script::RetrieveEntityFromContext(std::shared_ptr<Entity> entity, const char * stash_name) {
    duk_push_global_stash(this->context);
    if (duk_get_prop_string(this->context, -1, stash_name) != 0) {
        // only mutable properties are retrieved
        CB_Rect new_dim = GetPropertyRect(this->context, "dim");
        entity->dim.w = new_dim.w;
        entity->dim.h = new_dim.h;
        entity->time_scale = GetPropertyFloat(this->context, "time_scale");

        // set position (may result in collisions)
        entity->SetPosition(new_dim.x, new_dim.y);

        // special case: "destroyed flag"
        bool destroyed = GetPropertyBool(this->context, CB_SCRIPT_HIDDEN_DESTROYED);
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
    sprite->sprite_scale = GetPropertyFloat(this->context, "sprite_scale");
    sprite->tile_height = GetPropertyInt(this->context, "tile_height");
    sprite->tile_width = GetPropertyInt(this->context, "tile_width");
    sprite->tile_offset_x = GetPropertyInt(this->context, "tile_offset_x");
    sprite->tile_offset_y = GetPropertyInt(this->context, "tile_offset_y");
    sprite->flip_x = GetPropertyBool(this->context, "flip_x");
    sprite->flip_y = GetPropertyBool(this->context, "flip_y");
    duk_get_prop_string(this->context, -1, "frame");
    int current_frame = GetPropertyInt(this->context, "current");
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
        if (duk_pcall_method(this->context, 1) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entities
            duk_pop_2(this->context);
            RetrieveEntityFromContext(entity, CB_SCRIPT_ENTITY_THIS);
            RetrieveEntityFromContext(other_entity, other_entity_name.c_str());
        } else {
            LOG_ERR("Script::CallOnCollision oncollision() call failed in " + this->script_path + " - " +
                    std::string(duk_safe_to_string(this->context, -1)));
            duk_pop_2(this->context);
            // turn off the oncollision script so we don't get caught in an infinite loop of errors
            this->global_oncollision = false;
        }
    }
}

void Script::CallStart(std::shared_ptr<Entity> entity) {
    if (this->global_start) {
        // setup call to global start script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_START);
        CreateEntityInContext(entity, CB_SCRIPT_ENTITY_THIS);
        if (duk_pcall_method(this->context, 0) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entity
            duk_pop_2(this->context);
            RetrieveEntityFromContext(entity, CB_SCRIPT_ENTITY_THIS);
        } else {
            LOG_ERR("Script::CallStart start() call failed in " + this->script_path + " - " +
                    std::string(duk_safe_to_string(this->context, -1)));
            duk_pop_2(this->context);
            // turn off the start script so we don't get caught in an infinite loop of errors
            this->global_start = false;
        }
    }
}

void Script::CallUpdate(std::shared_ptr<Entity> entity, float delta_time) {
    if (this->global_update) {
        // setup call to global update script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_UPDATE);
        CreateEntityInContext(entity, CB_SCRIPT_ENTITY_THIS);
        duk_push_number(this->context, delta_time);
        if (duk_pcall_method(this->context, 1) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entity
            duk_pop_2(this->context);
            RetrieveEntityFromContext(entity, CB_SCRIPT_ENTITY_THIS);
        } else {
            LOG_ERR("Script::CallUpdate update() call failed in " + this->script_path + " - " +
                    std::string(duk_safe_to_string(this->context, -1)));
            duk_pop_2(this->context);
            // turn off the update script so we don't get caught in an infinite loop of errors
            this->global_update = false;
        }
    }
}
}
}