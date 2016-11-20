#include <cstring>

#include <cb/critterbits.hpp>
#include <cb/scripting/scriptsupport.hpp>

namespace Critterbits {
namespace Scripting {

entity_id_t next_callback_id = CB_ENTITY_ID_FIRST;

bool Script::CallCallback(std::shared_ptr<Entity> entity, const CB_ScriptCallback & callback) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(this->context);
    bool retval = false;
    duk_push_global_stash(this->context);
    duk_get_prop_string(this->context, -1, CB_SCRIPT_CALLBACK_STASH);
    if (duk_is_object(this->context, -1)) {
        duk_get_prop_index(this->context, -1, callback.callback_id);
        if (duk_is_ecmascript_function(this->context, -1)) {
            CreateEntityInContext(this->context, entity);
            if (duk_pcall_method(this->context, 0) == DUK_EXEC_SUCCESS) {
                if (!callback.once) {
                    retval = duk_get_boolean(this->context, -1);
                }
            } else {
                LOG_ERR("Script::CallCallback call failed in " + this->script_path + " - " +
                        std::string(duk_safe_to_string(this->context, -1)));
            }
        }
        duk_pop(this->context);
        if (!retval) {
            duk_del_prop_index(this->context, -1, callback.callback_id);
        }
    }
    duk_pop_2(this->context);
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(this->context);
    return retval;
}

void Script::DiscoverGlobals() {
    LOG_INFO("Script::DiscoverGlobals starting for " + this->script_path);
    if (this->context == nullptr) {
        return;
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(this->context);

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
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void Script::CallOnCollision(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> other_entity) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(this->context);
    if (this->global_oncollision) {
        // setup call to global oncollision script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_ONCOLLISION);
        CreateEntityInContext(this->context, entity);
        CreateEntityInContext(this->context, other_entity);
        if (duk_pcall_method(this->context, 1) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entities
            duk_pop_2(this->context);
            this->PostCallRetrieveAllEntities();
        } else {
            LOG_ERR("Script::CallOnCollision oncollision() call failed in " + this->script_path + " - " +
                    std::string(duk_safe_to_string(this->context, -1)));
            duk_pop_2(this->context);
            // turn off the oncollision script so we don't get caught in an infinite loop of errors
            this->global_oncollision = false;
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void Script::CallStart(std::shared_ptr<Entity> entity) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(this->context);
    if (this->global_start) {
        // setup call to global start script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_START);
        CreateEntityInContext(this->context, entity);
        if (duk_pcall_method(this->context, 0) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entity
            duk_pop_2(this->context);
            this->PostCallRetrieveAllEntities();
        } else {
            LOG_ERR("Script::CallStart start() call failed in " + this->script_path + " - " +
                    std::string(duk_safe_to_string(this->context, -1)));
            duk_pop_2(this->context);
            // turn off the start script so we don't get caught in an infinite loop of errors
            this->global_start = false;
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void Script::CallUpdate(std::shared_ptr<Entity> entity, float delta_time) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(this->context);
    if (this->global_update) {
        // setup call to global update script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_UPDATE);
        CreateEntityInContext(this->context, entity);
        duk_push_number(this->context, delta_time);
        if (duk_pcall_method(this->context, 1) == DUK_EXEC_SUCCESS) {
            // clean up
            duk_pop_2(this->context);

            // check callbacks
            for (auto it = this->callbacks.begin(); it != this->callbacks.end();) {
                CB_ScriptCallback * callback = it->get();
                if (auto callback_owner = callback->owner.lock()) {
                    if (callback_owner->entity_id == entity->entity_id) {
                        // update time elapsed and check if we've passed interval
                        callback->accrued += delta_time * 1000;
                        if (callback->accrued >= callback->delay) {
                            if (this->CallCallback(entity, *callback)) {
                                // callback continuing, reset timer
                                callback->accrued = 0;
                            } else {
                                // callback isn't interested in continuing
                                it = this->callbacks.erase(it);
                                continue;
                            }
                        }
                    }
                } else {
                    // callbacks's owner no longer exists, dump it
                    it = this->callbacks.erase(it);
                    continue;
                }
                it++;
            }

            // pull any changes to entities
            this->PostCallRetrieveAllEntities();
        } else {
            LOG_ERR("Script::CallUpdate update() call failed in " + this->script_path + " - " +
                    std::string(duk_safe_to_string(this->context, -1)));
            duk_pop_2(this->context);
            // turn off the update script so we don't get caught in an infinite loop of errors
            this->global_update = false;
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void Script::PostCallRetrieveAllEntities() {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    // iterate all entities in the global stash
    duk_push_global_stash(this->context);
    if (duk_get_prop_string(this->context, -1, CB_SCRIPT_ENTITY_STASH_ARRAY)) {
        int num_ents = duk_get_length(context, -1);
        for (int i = 0; i < num_ents; i++) {
            if (duk_get_prop_index(context, -1, i) != 0) {
                if (duk_is_object(context, -1)) {
                    duk_get_prop_string(context, -1, CB_SCRIPT_HIDDEN_ENTITYID);
                    entity_id_t eid = duk_get_int(context, -1);
                    duk_pop(context);
                    std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(eid);
                    if (entity != nullptr) {
                        RetrieveEntityFromContextAt(this->context, entity);
                    }
                }
            } else {
                LOG_ERR("CreateEntityInContext error retrieving property at index " + std::to_string(i));
            }
            duk_pop(context);
        }
    }
    duk_pop_2(this->context); // stash and array
    ClearEntitiesInContext(this->context);
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

void Script::QueueCallback(std::unique_ptr<CB_ScriptCallback> callback) {
    this->callbacks.push_back(std::move(callback));
}
}
}