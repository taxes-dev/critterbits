#include <cstring>

#include <critterbits.h>
#include <scripting/cbscriptsupport.hpp>

namespace Critterbits {
namespace Scripting {

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

void Script::CallOnCollision(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> other_entity) {
    if (this->global_oncollision) {
        std::string this_entity_name{CB_SCRIPT_ENTITY_NAME(entity)};
        std::string other_entity_name{CB_SCRIPT_ENTITY_NAME(other_entity)};

        // setup call to global oncollision script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_ONCOLLISION);
        CreateEntityInContext(this->context, entity, this_entity_name.c_str());
        CreateEntityInContext(this->context, other_entity, other_entity_name.c_str());
        if (duk_pcall_method(this->context, 1) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entities
            duk_pop_2(this->context);
            RetrieveEntityFromContext(this->context, entity, this_entity_name.c_str());
            RetrieveEntityFromContext(this->context, other_entity, other_entity_name.c_str());
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
        std::string this_entity_name{CB_SCRIPT_ENTITY_NAME(entity)};

        // setup call to global start script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_START);
        CreateEntityInContext(this->context, entity, this_entity_name.c_str());
        if (duk_pcall_method(this->context, 0) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entity
            duk_pop_2(this->context);
            RetrieveEntityFromContext(this->context, entity, this_entity_name.c_str());
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
        std::string this_entity_name{CB_SCRIPT_ENTITY_NAME(entity)};

        // setup call to global update script
        duk_push_global_object(this->context);
        duk_get_prop_string(this->context, -1, CB_SCRIPT_GLOBAL_UPDATE);
        CreateEntityInContext(this->context, entity, this_entity_name.c_str());
        duk_push_number(this->context, delta_time);
        if (duk_pcall_method(this->context, 1) == DUK_EXEC_SUCCESS) {
            // clean up and pull any changes to the entity
            duk_pop_2(this->context);
            RetrieveEntityFromContext(this->context, entity, this_entity_name.c_str());
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