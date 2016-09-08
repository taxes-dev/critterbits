#pragma once
#ifndef CBSCRIPTING_H
#define CBSCRIPTING_H

#include <memory>
#include <string>
#include <vector>

#include <duktape/duktape.h>

#include <cbentity.h>
#include <cbsprite.h>

#define CB_SCRIPT_PATH "scripts"
#define CB_SCRIPT_EXT ".js"

#define CB_SCRIPT_GLOBAL_ONCOLLISION "oncollision"
#define CB_SCRIPT_GLOBAL_START "start"
#define CB_SCRIPT_GLOBAL_UPDATE "update"

#define CB_SCRIPT_ENTITY_THIS "entity_this"
#define CB_SCRIPT_ENTITY_NAME(e) "entity_" + std::to_string(e->entity_id)

#define CB_SCRIPT_HIDDEN_DESTROYED                                                                                     \
    "\xff"                                                                                                             \
    "destroyed"
#define CB_SCRIPT_HIDDEN_ENTITYID                                                                                      \
    "\xff"                                                                                                             \
    "entity_id"

namespace Critterbits {
namespace Scripting {

class Script {
    friend class ScriptEngine;

  public:
    std::string script_path;
    std::string script_name;

    void CallOnCollision(std::shared_ptr<Entity>, std::shared_ptr<Entity>);
    void CallStart(std::shared_ptr<Entity>);
    void CallUpdate(std::shared_ptr<Entity>, float);

  private:
    duk_context * context{nullptr};
    bool global_oncollision{false};
    bool global_start{false};
    bool global_update{false};

    void CreateEntityInContext(std::shared_ptr<Entity>, const char *);
    void ExtendEntityWithSprite(std::shared_ptr<Sprite>);
    void DiscoverGlobals();
    void RetrieveEntityFromContext(std::shared_ptr<Entity>, const char *);
    void RetrieveSpriteFromContext(std::shared_ptr<Sprite>);
};

class ScriptEngine {
  public:
    ScriptEngine();
    ~ScriptEngine();
    std::shared_ptr<Script> GetScriptHandle(const std::string &);
    std::shared_ptr<Script> LoadScript(const std::string &);

  private:
    duk_context * context{nullptr};
    std::vector<std::shared_ptr<Script>> loaded_scripts;

    void AddCommonScriptingFunctions(duk_context *);

    ScriptEngine(const ScriptEngine &) = delete;
    ScriptEngine(ScriptEngine &&) = delete;
    void operator=(ScriptEngine const &) = delete;
};
}
}
#endif