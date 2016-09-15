#include <critterbits.hpp>
#include <scripting/cbscriptsupport.hpp>

namespace Critterbits {
namespace Scripting {

namespace {
/*
 * Generic fatal error handler for duktape
 */
void duktape_fatal_error(duk_context * ctx, duk_errcode_t code, const char * msg) {
    LOG_ERR("duktape runtime encountered fatal error " + std::to_string(code) + " " + std::string(msg));
    abort();
}

/*
* Functions callable from JavaScript code
*/
duk_ret_t find_entities_by_tag(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    int nargs = duk_get_top(context);
    std::vector<std::string> tags;
    for (int i = 0; i < nargs; i++) {
        tags.push_back(std::string(duk_get_string(context, i)));
    }
    int arr_idx = duk_push_array(context);
    int prop_idx = 0;
    for (auto & tag : tags) {
        for (auto & entity : Engine::GetInstance().FindEntitiesByTag(tag)) {
            CreateEntityInContext(context, entity);
            duk_put_prop_index(context, arr_idx, prop_idx++);
        }
    }
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

duk_ret_t is_key_pressed(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    int key_code = duk_get_int(context, 0);
    bool key_pressed = Engine::GetInstance().input.IsKeyPressed((CB_KeyCode)key_code);
    duk_push_boolean(context, key_pressed ? 1 : 0);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

duk_ret_t spawn_sprite(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    if (duk_is_string(context, 0)) {
        QueuedSprite qsprite;
        qsprite.name = duk_get_string(context, 0);
        if (duk_is_object(context, 1)) {
            qsprite.at.x = GetPropertyInt(context, "x", 1);
            qsprite.at.y = GetPropertyInt(context, "y", 1);
        }
        if (Engine::GetInstance().scenes.current_scene != nullptr) {
            Engine::GetInstance().scenes.current_scene->sprites.QueueSprite(qsprite);
        }
    }
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
    return 0;
}

duk_ret_t viewport_follow(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    bool success = false;
    if (duk_is_object(context, 0)) {
        duk_get_prop_string(context, 0, CB_SCRIPT_HIDDEN_ENTITYID);
        if (duk_is_undefined(context, -1) == 0) {
            entity_id_t entity_id = duk_get_uint(context, -1);
            std::shared_ptr<Entity> entity = Engine::GetInstance().FindEntityById(entity_id);
            if (entity != nullptr) {
                Engine::GetInstance().viewport->SetEntityToFollow(entity);
                success = true;
            }
        }
        duk_pop(context);
    }
    duk_push_boolean(context, success);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}
/*
 * End support functions
 */
}

ScriptEngine::ScriptEngine() {
    this->context = duk_create_heap(NULL, NULL, NULL, NULL, duktape_fatal_error);
    if (this->context == nullptr) {
        LOG_ERR("ScriptEngine::ScriptEngine unable to create duktape context");
    }
}

ScriptEngine::~ScriptEngine() {
    if (this->context != nullptr) {
        duk_destroy_heap(this->context);
    }
}

#define CB_PUT_KEYCODE(k)                                                                                              \
    duk_push_int(context, SDLK_##k);                                                                                   \
    duk_put_prop_string(context, -2, #k);

void ScriptEngine::AddCommonScriptingFunctions(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    duk_push_global_object(context);

    // input manager
    duk_push_object(context); // input
    duk_push_object(context); // key_codes
    CB_PUT_KEYCODE(UP);
    CB_PUT_KEYCODE(DOWN);
    CB_PUT_KEYCODE(LEFT);
    CB_PUT_KEYCODE(RIGHT);
    duk_put_prop_string(context, -2, "key_codes");
    duk_push_c_function(context, is_key_pressed, 1);
    duk_put_prop_string(context, -2, "is_key_pressed");
    duk_put_prop_string(context, -2, "input");

    // viewport
    std::shared_ptr<Viewport> viewport = Engine::GetInstance().viewport;
    duk_push_object(context); // viewport
    PushPropertyRect(context, "dim", viewport->dim);
    duk_push_c_function(context, viewport_follow, 1);
    duk_put_prop_string(context, -2, "follow");
    duk_put_prop_string(context, -2, "viewport");

    // global functions
    duk_push_c_function(context, find_entities_by_tag, DUK_VARARGS);
    duk_put_prop_string(context, -2, "find_by_tag");
    duk_push_c_function(context, spawn_sprite, 2);
    duk_put_prop_string(context, -2, "spawn");

    duk_pop(context); // global
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

std::shared_ptr<Script> ScriptEngine::GetScriptHandle(const std::string & script_name) {
    for (auto & script : this->loaded_scripts) {
        if (script->script_name == script_name) {
            return script;
        }
    }
    return nullptr;
}

std::shared_ptr<Script> ScriptEngine::LoadScript(const std::string & script_name) {
    if (this->context == nullptr) {
        LOG_ERR("ScriptEngine::LoadScript no scripting runtime");
        return nullptr;
    }
    std::string script_path =
        Engine::GetInstance().config->asset_path + CB_SCRIPT_PATH + PATH_SEP + script_name + CB_SCRIPT_EXT;
    LOG_INFO("ScriptEngine::LoadScript about to load " + script_path);
    if (!FileExists(script_path)) {
        LOG_INFO("ScriptEngine::LoadScript script not found");
        return nullptr;
    }

    std::shared_ptr<Script> new_script{std::make_shared<Script>()};
    new_script->script_name = script_name;
    new_script->script_path = script_path;

    // create a new context
    duk_push_thread_new_globalenv(this->context);
    new_script->context = duk_get_context(this->context, -1);
    if (new_script->context == nullptr) {
        LOG_ERR("ScriptEngine::LoadScript unable to create new script context");
        return nullptr;
    }

    // add common globals to the new context
    this->AddCommonScriptingFunctions(new_script->context);

    // load associated script file
    if (duk_peval_file(new_script->context, script_path.c_str()) != 0) {
        const char * error = duk_safe_to_string(new_script->context, -1);
        LOG_ERR("ScriptEngine::LoadScript unable to compile script " + std::string(error));
        return false;
    }
    duk_pop(new_script->context); // ignore result of peval

    // prepare the script object
    new_script->DiscoverGlobals();

    this->loaded_scripts.push_back(new_script);
    return new_script;
}
}
}