#include <cstring>
#include <cstdlib>

#include <critterbits.hpp>
#include <scripting/cbscriptsupport.hpp>

namespace Critterbits {
namespace Scripting {

namespace {
/*
* Functions callable from JavaScript code
*/
duk_ret_t mark_entity_destroyed(duk_context * context) {
    duk_push_this(context);
    PushPropertyBool(context, CB_SCRIPT_HIDDEN_DESTROYED, true);
    duk_pop(context);
    return 0;
}

/*
 * Sprite entity push/pop
 */
void ExtendEntityWithSprite(duk_context * context, std::shared_ptr<Sprite> sprite) {
    PushPropertyString(context, "entity_type", "sprite");
    PushPropertyFloat(context, "sprite_scale", sprite->sprite_scale);
    PushPropertyInt(context, "tile_height", sprite->tile_height);
    PushPropertyInt(context, "tile_width", sprite->tile_width);
    PushPropertyInt(context, "tile_offset_x", sprite->tile_offset_x);
    PushPropertyInt(context, "tile_offset_y", sprite->tile_offset_y);
    PushPropertyBool(context, "flip_x", sprite->flip_x);
    PushPropertyBool(context, "flip_y", sprite->flip_y);

    duk_push_object(context); // frame
    PushPropertyInt(context, "current", sprite->GetFrame());
    PushPropertyInt(context, "count", sprite->GetFrameCount());
    duk_put_prop_string(context, -2, "frame");
}


void RetrieveSpriteFromContext(duk_context * context, std::shared_ptr<Sprite> sprite, int stack_index) {
    sprite->sprite_scale = GetPropertyFloat(context, "sprite_scale", stack_index);
    sprite->tile_height = GetPropertyInt(context, "tile_height", stack_index);
    sprite->tile_width = GetPropertyInt(context, "tile_width", stack_index);
    sprite->tile_offset_x = GetPropertyInt(context, "tile_offset_x", stack_index);
    sprite->tile_offset_y = GetPropertyInt(context, "tile_offset_y", stack_index);
    sprite->flip_x = GetPropertyBool(context, "flip_x", stack_index);
    sprite->flip_y = GetPropertyBool(context, "flip_y", stack_index);
    duk_get_prop_string(context, stack_index, "frame");
    int current_frame = GetPropertyInt(context, "current", -1);
    sprite->SetFrame(current_frame);
    duk_pop(context); // frame
}
/* 
 * End of support functions
 */
}

void ClearEntitiesInContext(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    duk_push_global_stash(context);
    duk_del_prop_string(context, -1, CB_SCRIPT_ENTITY_STASH_ARRAY);
    duk_pop(context);
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void CreateEntityInContext(duk_context * context, std::shared_ptr<Entity> entity) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    duk_push_global_stash(context); // will save to the global stash when we're done

    // create array if it doesn't exist
    duk_get_prop_string(context, -1, CB_SCRIPT_ENTITY_STASH_ARRAY);
    if (!duk_is_array(context, -1)) {
        duk_pop(context);
        duk_push_array(context);
        duk_put_prop_string(context, -2, CB_SCRIPT_ENTITY_STASH_ARRAY);
        duk_get_prop_string(context, -1, CB_SCRIPT_ENTITY_STASH_ARRAY);
    }

    // swap the stash array with the global stash and pop it off
    duk_swap_top(context, -2);
    duk_pop(context);

    // are we already on the stash?
    int found_stash_idx = -1;
    int num_props = duk_get_length(context, -1);
    for (int i = 0; i < num_props; i++) {
        if (duk_get_prop_index(context, -1, i) != 0) {
            if (duk_is_object(context, -1)) {
                duk_get_prop_string(context, -1, CB_SCRIPT_HIDDEN_ENTITYID);
                entity_id_t eid = duk_get_int(context, -1);
                duk_pop_2(context);
                if (eid == entity->entity_id) {
                    found_stash_idx = i;
                    break;
                }
            } else {
                duk_pop(context);
            }
        } else {
            LOG_ERR("CreateEntityInContext error retrieving property at index " + std::to_string(i));
            duk_pop(context);
        }
    }

    if (found_stash_idx == -1) {
        // doesn't exist, create a new one
        duk_push_object(context); // root
        PushPropertyRect(context, "dim", entity->dim);
        PushPropertyEntityId(context, entity->entity_id);
        PushPropertyString(context, "tag", entity->tag);
        PushPropertyFloat(context, "time_scale", entity->time_scale);

        duk_push_c_function(context, mark_entity_destroyed, 0);
        duk_put_prop_string(context, -2, "destroy");

        if (entity->GetEntityType() == CBE_SPRITE) {
            ExtendEntityWithSprite(context, std::dynamic_pointer_cast<Sprite>(entity));
        }

        // save to the end of the stash array so we can grab it after method call
        duk_put_prop_index(context, -2, num_props);

        // now put the entity back on the top of the stack
        duk_get_prop_index(context, -1, num_props);
    } else {
        // exists, put it on the stack
        duk_get_prop_index(context, -1, found_stash_idx);
    }

    // swap the entity with the stash array and then pop the stash array off
    duk_swap_top(context, -2);
    duk_pop(context);

    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
}

void RetrieveEntityFromContext(duk_context * context, std::shared_ptr<Entity> entity) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    duk_push_global_stash(context);
    if (duk_get_prop_string(context, -1, CB_SCRIPT_ENTITY_STASH_ARRAY)) {
        // got the array, now find the entity
        int found_stash_idx = -1;
        int num_props = duk_get_length(context, -1);
        for (int i = 0; i < num_props; i++) {
            if (duk_get_prop_index(context, -1, i) != 0) {
                if (duk_is_object(context, -1)) {
                    duk_get_prop_string(context, -1, CB_SCRIPT_HIDDEN_ENTITYID);
                    entity_id_t eid = duk_get_int(context, -1);
                    duk_pop_2(context);
                    if (eid == entity->entity_id) {
                        found_stash_idx = i;
                        break;
                    }
                } else {
                    duk_pop(context);
                }
            } else {
                LOG_ERR("RetrieveEntityFromContext error retrieving property at index " + std::to_string(i));
                duk_pop(context);
            }
        }

        if (found_stash_idx > -1) {
            duk_get_prop_index(context, -1, found_stash_idx);

            RetrieveEntityFromContextAt(context, entity);

            duk_pop(context); // entity
        } else {
            LOG_ERR("Script::RetrieveEntityFromContext unable to find entity in stash array");
        }
    } else {
        LOG_ERR("Script::RetrieveEntityFromContext unable to retrieve stash array");
    }
    duk_pop_2(context); // stash and array
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void RetrieveEntityFromContextAt(duk_context * context, std::shared_ptr<Entity> entity, int stack_index) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    if (duk_is_object(context, stack_index)) {
        // only mutable properties are retrieved
        CB_Rect new_dim = GetPropertyRect(context, "dim", stack_index);
        entity->dim.w = new_dim.w;
        entity->dim.h = new_dim.h;
        entity->time_scale = GetPropertyFloat(context, "time_scale", stack_index);

        // set position (may result in collisions)
        entity->SetPosition(new_dim.x, new_dim.y);

        // special case: "destroyed flag"
        bool destroyed = GetPropertyBool(context, CB_SCRIPT_HIDDEN_DESTROYED, stack_index);
        if (destroyed) {
            entity->MarkDestroy();
        }

        if (entity->GetEntityType() == CBE_SPRITE) {
            RetrieveSpriteFromContext(context, std::dynamic_pointer_cast<Sprite>(entity), stack_index);
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}
}
}