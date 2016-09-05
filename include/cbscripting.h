#pragma once
#ifndef CBSCRIPTING_H
#define CBSCRIPTING_H

#include <memory>
#include <string>
#include <vector>

#include <duktape/duktape.h>

#define CB_SCRIPT_PATH "scripts"
#define CB_SCRIPT_EXT ".js"

namespace Critterbits {

class Script {
    friend class ScriptEngine;

  public:
    std::string script_path;
    std::string script_name;

    void CallUpdate(std::shared_ptr<Entity>, float);

  private:
    duk_context * context = nullptr;
    bool global_update = false;

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
    duk_context * context = nullptr;
    std::vector<std::shared_ptr<Script>> loaded_scripts;

    void AddCommonScriptingFunctions(duk_context *);

    ScriptEngine(const ScriptEngine &) = delete;
    ScriptEngine(ScriptEngine &&) = delete;
    void operator=(ScriptEngine const &) = delete;
};
}

#endif