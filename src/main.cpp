#include <string>

#include "cbengine.h"
#include "cblogging.h"

int main(int argc, char ** argv) {
    // create engine
    Critterbits::Engine engine;

    // determine asset path and load configuration
    std::string asset_path = CB_DEFAULT_ASSET_PATH;
    if (argc > 1) {
        asset_path = std::string(argv[1]);
    }
    Critterbits::EngineConfiguration config(asset_path);
    engine.config = config;

    // run (blocks until window closed)
    return engine.Run();
}