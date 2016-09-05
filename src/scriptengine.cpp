#include <critterbits.h>

#include <duktape/duktape.h>

namespace Critterbits {

namespace {
/*
 * Generic fatal error handler for duktape
 */
void duktape_fatal_error(duk_context * ctx, duk_errcode_t code, const char * msg) {
    LOG_ERR("duktape runtime encountered fatal error " + std::to_string(code) + " " + std::string(msg));
}

/*
 * Support functions for ScriptEngine::AddCommonScriptingFunctions()
 */

duk_ret_t is_key_pressed(duk_context * context) {
    int key_code = duk_get_int(context, 0);
    bool key_pressed = Engine::GetInstance().input.IsKeyPressed(key_code);
    duk_push_boolean(context, key_pressed);
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

void ScriptEngine::AddCommonScriptingFunctions(duk_context * context) {
    duk_push_global_object(context);

    // input manager
    duk_push_object(context); // input
    duk_push_object(context); // key_codes
    duk_push_int(context, SDLK_UP);
    duk_put_prop_string(context, -2, "UP");
    duk_push_int(context, SDLK_DOWN);
    duk_put_prop_string(context, -2, "DOWN");
    duk_push_int(context, SDLK_LEFT);
    duk_put_prop_string(context, -2, "LEFT");
    duk_push_int(context, SDLK_RIGHT);
    duk_put_prop_string(context, -2, "RIGHT");
    duk_put_prop_string(context, -2, "key_codes");
    duk_push_c_function(context, is_key_pressed, 1);
    duk_put_prop_string(context, -2, "is_key_pressed");
    duk_put_prop_string(context, -2, "input");

    duk_dump_context_stdout(context);
    duk_pop(context); // global
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
        LOG_INFO("ScriptEngine::Loadscript script not found");
        return nullptr;
    }

    std::shared_ptr<Script> new_script(new Script());
    new_script->script_name = script_name;
    new_script->script_path = script_path;
    duk_push_thread_new_globalenv(this->context);
    new_script->context = duk_get_context(this->context, 0);
    if (new_script->context == nullptr) {
        LOG_ERR("ScriptEngine::LoadScript unable to create new script context");
        return nullptr;
    }
    this->AddCommonScriptingFunctions(new_script->context);
    if (duk_peval_file(new_script->context, script_path.c_str()) != 0) {
        const char * error = duk_safe_to_string(new_script->context, 0);
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