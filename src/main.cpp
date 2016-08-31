#include <string>

#include <critterbits.h>

int main(int argc, char ** argv) {
    // determine asset path and load configuration
    std::string asset_path = CB_DEFAULT_ASSET_PATH;
    if (argc > 1) {
        asset_path = std::string(argv[1]);
    }
    Critterbits::EngineConfiguration config(asset_path);

    // create engine
    Critterbits::Engine engine(config);

    // run (blocks until window closed)
    return engine.Run();
}