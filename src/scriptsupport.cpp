#include <cstdlib>
#include <cstring>

#include <cb/critterbits.hpp>
#include <cb/scripting/scriptsupport.hpp>

namespace Critterbits {
namespace Scripting {

namespace {
/*
* Functions callable from JavaScript code
*/
duk_ret_t cancel_callback(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    entity_id_t callback_id = duk_require_int(context, 0);
    duk_push_global_stash(context);
    duk_get_prop_string(context, -1, CB_SCRIPT_CALLBACK_STASH);
    if (duk_is_object(context, -1)) {
        duk_del_prop_index(context, -1, callback_id);
    }
    duk_pop_2(context);
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

duk_ret_t queue_callback(duk_context * context, bool once) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    duk_push_this(context);
    entity_id_t entity_id = GetPropertyEntityId(context);
    entity_id_t callback_id = CB_ENTITY_ID_INVALID;
    duk_pop(context);
    std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(entity_id);
    if (entity != nullptr && entity->HasScript()) {
        int delay = duk_require_int(context, 0);
        if (duk_is_object(context, 1)) {
            std::unique_ptr<CB_ScriptCallback> callback{new CB_ScriptCallback()};
            callback->delay = delay;
            callback->once = once;
            callback->owner = entity;
            duk_push_global_stash(context);
            duk_get_prop_string(context, -1, CB_SCRIPT_CALLBACK_STASH);
            if (duk_is_object(context, -1) == 0) {
                // create the callback map
                duk_pop(context);
                duk_push_object(context);
                duk_put_prop_string(context, -2, CB_SCRIPT_CALLBACK_STASH);
                duk_get_prop_string(context, -1, CB_SCRIPT_CALLBACK_STASH);
            }
            duk_push_null(context);
            duk_swap(context, -1, 1);
            duk_put_prop_index(context, -2, callback->callback_id);
            duk_pop_2(context);
            callback_id = callback->callback_id;
            entity->script->QueueCallback(std::move(callback));
        }
    }
    if (callback_id == CB_ENTITY_ID_INVALID) {
        duk_push_undefined(context);
    } else {
        duk_push_int(context, callback_id);
    }
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

duk_ret_t delay_callback(duk_context * context) { return queue_callback(context, true); }

duk_ret_t interval_callback(duk_context * context) { return queue_callback(context, false); }

duk_ret_t mark_entity_destroyed(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    duk_push_this(context);
    PushPropertyBool(context, CB_SCRIPT_HIDDEN_DESTROYED, true);
    duk_pop(context);
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

duk_ret_t move_to(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    CB_Point end{0, 0};
    if (duk_is_object(context, 0)) {
        end.x = GetPropertyInt(context, "x", 0);
        end.y = GetPropertyInt(context, "y", 0);
    }
    float duration{0.f};
    if (duk_is_number(context, 1)) {
        int millis = duk_get_int(context, 1);
        duration = static_cast<float>(millis) / 1000.f;
    }
    Animation::TransformAlgorithm algorithm{Animation::TransformAlgorithm::Lerp};
    if (duk_is_string(context, 2)) {
        std::string alg_name{duk_get_string(context, 2)};
        if (alg_name == "ease-in") {
            algorithm = Animation::TransformAlgorithm::QuadEaseIn;
        } else if (alg_name != "lerp") {
            LOG_ERR("move_to: Unknown value for algorithm " + alg_name);
        }
    }
    if (duk_is_object(context, 3)) {
        //TODO:callback
    }
    duk_push_this(context);
    entity_id_t entity_id = GetPropertyEntityId(context);
    duk_pop(context);
    std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(entity_id);
    if (entity != nullptr && entity->GetEntityType() == EntityType::Sprite && duration > 0.f) {
        LOG_INFO("starting animation " + std::to_string(duration) + " " + end.to_string());
        std::shared_ptr<Animation::TranslateAnimation> translate = std::make_shared<Animation::TranslateAnimation>(algorithm, duration, entity->dim.xy(), end);
        translate->Play();
        std::dynamic_pointer_cast<Sprite>(entity)->animations.push_back(std::move(translate));
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

duk_ret_t play_animation(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    std::string anim_name{duk_get_string(context, 0)};
    bool stop_others = true;
    if (duk_is_boolean(context, 1)) {
        stop_others = duk_get_boolean(context, 1) != 0;
    }
    duk_push_this(context);
    entity_id_t entity_id = GetPropertyEntityId(context);
    duk_pop(context);
    std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(entity_id);
    if (entity != nullptr) {
        std::shared_ptr<Sprite> sprite = std::dynamic_pointer_cast<Sprite>(entity);
        for (auto & anim : sprite->animations) {
            if (anim->name == anim_name) {
                anim->Play();
            } else if (stop_others) {
                anim->Stop();
            }
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

duk_ret_t stop_animation(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    std::string anim_name{duk_get_string(context, 0)};
    duk_push_this(context);
    entity_id_t entity_id = GetPropertyEntityId(context);
    duk_pop(context);
    std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(entity_id);
    if (entity != nullptr) {
        std::shared_ptr<Sprite> sprite = std::dynamic_pointer_cast<Sprite>(entity);
        for (auto & anim : sprite->animations) {
            if (anim->name == anim_name) {
                anim->Stop();
            }
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

duk_ret_t stop_all_animation(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    duk_push_this(context);
    entity_id_t entity_id = GetPropertyEntityId(context);
    duk_pop(context);
    std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(entity_id);
    if (entity != nullptr) {
        std::shared_ptr<Sprite> sprite = std::dynamic_pointer_cast<Sprite>(entity);
        for (auto & anim : sprite->animations) {
            if (anim->name.length() > 0) { //TODO:better definition of user-defined animations
                anim->Stop();
            }
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

/*
 * Sprite entity push/pop
 */
void ExtendEntityWithSprite(duk_context * context, std::shared_ptr<Sprite> sprite) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    PushPropertyString(context, "entity_type", "sprite");
    PushPropertyFloat(context, "sprite_scale", sprite->sprite_scale);
    PushPropertyInt(context, "tile_height", sprite->tile_height);
    PushPropertyInt(context, "tile_width", sprite->tile_width);
    PushPropertyInt(context, "tile_offset_x", sprite->tile_offset_x);
    PushPropertyInt(context, "tile_offset_y", sprite->tile_offset_y);
    PushPropertyBool(context, "flip_x", sprite->flip_x);
    PushPropertyBool(context, "flip_y", sprite->flip_y);
    PushPropertyColor(context, "tint", sprite->tint_and_opacity);
    PushPropertyFloat(context, "opacity", static_cast<float>(sprite->tint_and_opacity.a) / 255.0f);
    PushPropertyFunction(context, "move_to", move_to, 4);

    duk_push_object(context); // frame
    PushPropertyInt(context, "current", sprite->GetFrame());
    PushPropertyInt(context, "count", sprite->GetFrameCount());
    duk_put_prop_string(context, -2, "frame");

    duk_push_object(context); // animation
    PushPropertyEntityId(context, sprite->entity_id);
    PushPropertyFunction(context, "play", play_animation, 2);
    PushPropertyFunction(context, "stop", stop_animation, 1);
    PushPropertyFunction(context, "stop_all", stop_all_animation);
    duk_put_prop_string(context, -2, "animation");

    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void RetrieveSpriteFromContext(duk_context * context, std::shared_ptr<Sprite> sprite, int stack_index) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    sprite->sprite_scale = GetPropertyFloat(context, "sprite_scale", stack_index);
    sprite->tile_height = GetPropertyInt(context, "tile_height", stack_index);
    sprite->tile_width = GetPropertyInt(context, "tile_width", stack_index);
    sprite->tile_offset_x = GetPropertyInt(context, "tile_offset_x", stack_index);
    sprite->tile_offset_y = GetPropertyInt(context, "tile_offset_y", stack_index);
    sprite->flip_x = GetPropertyBool(context, "flip_x", stack_index);
    sprite->flip_y = GetPropertyBool(context, "flip_y", stack_index);
    sprite->tint_and_opacity = GetPropertyColor(context, "tint", stack_index);
    float opacity = GetPropertyFloat(context, "opacity", stack_index);
    sprite->tint_and_opacity.a = Clamp(static_cast<int>(255.0f * opacity), 0, 255);
    duk_get_prop_string(context, stack_index, "frame");
    int current_frame = GetPropertyInt(context, "current", -1);
    sprite->SetFrame(current_frame);
    duk_pop(context); // frame
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
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
        duk_push_object(context); // dim
        PushPropertyInt(context, "w", entity->dim.w);
        PushPropertyInt(context, "h", entity->dim.h);
        duk_put_prop_string(context, -2, "dim");
        PushPropertyPoint(context, "pos", entity->dim.xy());
        PushPropertyEntityId(context, entity->entity_id);
        PushPropertyString(context, "tag", entity->tag);
        PushPropertyFloat(context, "time_scale", entity->time_scale);
        PushPropertyFunction(context, "destroy", mark_entity_destroyed);
        PushPropertyFunction(context, "delay", delay_callback, 2);
        PushPropertyFunction(context, "interval", interval_callback, 2);
        PushPropertyFunction(context, "cancel", cancel_callback, 1);

        if (entity->GetEntityType() == EntityType::Sprite) {
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
        CB_Point new_dim = GetPropertyRectDimOnly(context, "dim", stack_index);
        entity->dim.wh(new_dim);
        CB_Point new_pos = GetPropertyPoint(context, "pos", stack_index);
        entity->time_scale = GetPropertyFloat(context, "time_scale", stack_index);

        // set position (may result in collisions)
        entity->SetPosition(new_pos.x, new_pos.y);

        // special case: "destroyed flag"
        bool destroyed = GetPropertyBool(context, CB_SCRIPT_HIDDEN_DESTROYED, stack_index);
        if (destroyed) {
            entity->MarkDestroy();
        }

        if (entity->GetEntityType() == EntityType::Sprite) {
            RetrieveSpriteFromContext(context, std::dynamic_pointer_cast<Sprite>(entity), stack_index);
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}
}
}