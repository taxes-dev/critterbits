#include <scripting/cbscriptsupport.hpp>
#include <cblogging.h>
#include <cbsprite.h>

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


void RetrieveSpriteFromContext(duk_context * context, std::shared_ptr<Sprite> sprite) {
    sprite->sprite_scale = GetPropertyFloat(context, "sprite_scale");
    sprite->tile_height = GetPropertyInt(context, "tile_height");
    sprite->tile_width = GetPropertyInt(context, "tile_width");
    sprite->tile_offset_x = GetPropertyInt(context, "tile_offset_x");
    sprite->tile_offset_y = GetPropertyInt(context, "tile_offset_y");
    sprite->flip_x = GetPropertyBool(context, "flip_x");
    sprite->flip_y = GetPropertyBool(context, "flip_y");
    duk_get_prop_string(context, -1, "frame");
    int current_frame = GetPropertyInt(context, "current");
    sprite->SetFrame(current_frame);
    duk_pop(context); // frame
}
/* 
 * End of support functions
 */
}

void CreateEntityInContext(duk_context * context, std::shared_ptr<Entity> entity, const char * stash_name) {
    duk_push_global_stash(context); // will save to the global stash when we're done

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

    // save to the stash so we can grab it after method call
    duk_put_prop_string(context, -2, stash_name);

    // now put the entity back on the top of the stack
    duk_get_prop_string(context, -1, stash_name);
    duk_swap_top(context, -2);
    duk_pop(context);
}

void RetrieveEntityFromContext(duk_context * context, std::shared_ptr<Entity> entity, const char * stash_name) {
    duk_push_global_stash(context);
    if (duk_get_prop_string(context, -1, stash_name) != 0) {
        // only mutable properties are retrieved
        CB_Rect new_dim = GetPropertyRect(context, "dim");
        entity->dim.w = new_dim.w;
        entity->dim.h = new_dim.h;
        entity->time_scale = GetPropertyFloat(context, "time_scale");

        // set position (may result in collisions)
        entity->SetPosition(new_dim.x, new_dim.y);

        // special case: "destroyed flag"
        bool destroyed = GetPropertyBool(context, CB_SCRIPT_HIDDEN_DESTROYED);
        if (destroyed) {
            entity->MarkDestroy();
        }

        if (entity->GetEntityType() == CBE_SPRITE) {
            RetrieveSpriteFromContext(context, std::dynamic_pointer_cast<Sprite>(entity));
        }

        duk_pop(context); // "stash_name"
    } else {
        LOG_ERR("Script::RetrieveEntityFromContext unable to retrieve entity from stash");
    }
    duk_pop(context); // stash
}


}
}