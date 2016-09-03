#pragma once
#ifndef CBSCRIPTING_H
#define CBSCRIPTING_H

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

  private:
    duk_context * context = nullptr;
};

class ScriptEngine {
  public:
    ScriptEngine();
    ~ScriptEngine();
    bool LoadScript(const std::string &);

  private:
    duk_context * context = nullptr;
    std::vector<Script> loaded_scripts;

    ScriptEngine(const ScriptEngine &) = delete;
    ScriptEngine(ScriptEngine &&) = delete;
    void operator=(ScriptEngine const &) = delete;
};
}

#endif