#include <cb/critterbits.hpp>
#include <cb/scripting/scriptsupport.hpp>

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
duk_ret_t close_gui_panel(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    entity_id_t to_close = duk_get_uint(context, 0);
    bool closed = Engine::GetInstance().gui.ClosePanel(to_close);
    duk_push_boolean(context, closed);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

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

duk_ret_t is_direction_pressed(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    int direction = duk_get_int(context, 0);
    bool axis_pressed = Engine::GetInstance().input.IsAxisPressed(static_cast<InputDirection>(direction));
    duk_push_boolean(context, axis_pressed ? 1 : 0);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;    
}

duk_ret_t is_controller_direction_pressed(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    int direction = duk_get_int(context, 0);
    bool axis_pressed = Engine::GetInstance().input.IsControllerAxisPressed(static_cast<InputDirection>(direction));
    duk_push_boolean(context, axis_pressed ? 1 : 0);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

duk_ret_t is_key_pressed(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    int key_code = duk_get_int(context, 0);
    bool key_pressed = Engine::GetInstance().input.IsKeyPressed(static_cast<CB_KeyCode>(key_code));
    duk_push_boolean(context, key_pressed);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

duk_ret_t is_key_pressed_once(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    int key_code = duk_get_int(context, 0);
    bool key_pressed = Engine::GetInstance().input.IsKeyPressedOnce(static_cast<CB_KeyCode>(key_code));
    duk_push_boolean(context, key_pressed);
    CB_SCRIPT_ASSERT_STACK_RETURN1_END(context);
    return 1;
}

duk_ret_t open_gui_panel(duk_context * context) {
    CB_SCRIPT_ASSERT_STACK_RETURN1_BEGIN(context);
    entity_id_t opened = CB_ENTITY_ID_INVALID;
    if (duk_is_string(context, 0)) {
        std::string panel_name{duk_get_string(context, 0)};
        if (!panel_name.empty()) {
            bool multiple = false;
            if (duk_is_boolean(context, 1)) {
                multiple = duk_get_boolean(context, 1);
            }
            opened = Engine::GetInstance().gui.OpenPanel(panel_name, multiple);
        }
    }
    duk_push_uint(context, opened);
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
        if (Engine::GetInstance().scenes.IsCurrentSceneActive()) {
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

void ScriptEngine::AddCommonScriptingFunctions(duk_context * context) const {
    CB_SCRIPT_ASSERT_STACK_CLEAN_BEGIN(context);
    duk_push_global_object(context);

    // input manager
    duk_push_object(context); // input
    duk_push_object(context); // key_codes
    CB_PUT_KEYCODE(UP);
    CB_PUT_KEYCODE(DOWN);
    CB_PUT_KEYCODE(LEFT);
    CB_PUT_KEYCODE(RIGHT);
    CB_PUT_KEYCODE(RETURN);
    CB_PUT_KEYCODE(ESCAPE);
    duk_put_prop_string(context, -2, "key_codes");
    duk_push_object(context); // direction
    PushPropertyInt(context, "LEFT", static_cast<int>(InputDirection::Left));
    PushPropertyInt(context, "RIGHT", static_cast<int>(InputDirection::Right));
    PushPropertyInt(context, "UP", static_cast<int>(InputDirection::Up));
    PushPropertyInt(context, "DOWN", static_cast<int>(InputDirection::Down));
    duk_put_prop_string(context, -2, "direction");
    PushPropertyFunction(context, "is_controller_direction_pressed", is_controller_direction_pressed, 1);
    PushPropertyFunction(context, "is_direction_pressed", is_direction_pressed, 1);
    PushPropertyFunction(context, "is_key_pressed", is_key_pressed, 1);
    PushPropertyFunction(context, "is_key_pressed_once", is_key_pressed_once, 1);
    duk_put_prop_string(context, -2, "input");

    // viewport
    std::shared_ptr<Viewport> viewport = Engine::GetInstance().viewport;
    duk_push_object(context); // viewport
    PushPropertyRect(context, "dim", viewport->dim);
    PushPropertyFunction(context, "follow", viewport_follow, 1);
    duk_put_prop_string(context, -2, "viewport");

    // global functions
    PushPropertyFunction(context, "close_gui", close_gui_panel, 1);
    PushPropertyFunction(context, "find_by_tag", find_entities_by_tag, DUK_VARARGS);
    PushPropertyFunction(context, "open_gui", open_gui_panel, 2);
    PushPropertyFunction(context, "spawn", spawn_sprite, 2);

    duk_pop(context); // global
    CB_SCRIPT_ASSERT_STACK_CLEAN_END(context);
}

std::shared_ptr<Script> ScriptEngine::GetScriptHandle(const std::string & script_path) const {
    for (auto & script : this->loaded_scripts) {
        if (script->script_path == script_path) {
            return script;
        }
    }
    return nullptr;
}

std::shared_ptr<Script> ScriptEngine::LoadScript(const std::string & script_path) {
    if (this->context == nullptr) {
        LOG_ERR("ScriptEngine::LoadScript no scripting runtime");
        return nullptr;
    }

    std::shared_ptr<Script> new_script = this->GetScriptHandle(script_path);
    if (new_script != nullptr) {
        LOG_INFO("ScriptEngine::LoadScript found existing script " + script_path);
        return new_script;
    }

    LOG_INFO("ScriptEngine::LoadScript about to load " + script_path);
    if (!Engine::GetInstance().GetResourceLoader()->ResourceExists(script_path)) {
        LOG_ERR("ScriptEngine::LoadScript script not found " + script_path);
        return nullptr;
    }

    new_script = std::make_shared<Script>();
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
    std::string * script_contents = nullptr;
    if (Engine::GetInstance().GetResourceLoader()->GetTextResourceContents(script_path, &script_contents) == false) {
        LOG_ERR("ScriptEngine::LoadScript unable to get script " + script_path);
        return false;
    }
    if (duk_peval_string_noresult(new_script->context, (*script_contents).c_str()) != 0) {
        const char * error = duk_safe_to_string(new_script->context, -1);
        LOG_ERR("ScriptEngine::LoadScript unable to compile script " + script_path + ", error was " + std::string(error));
        return false;
    }

    // prepare the script object
    new_script->DiscoverGlobals();

    this->loaded_scripts.push_back(new_script);
    return new_script;
}
}
}