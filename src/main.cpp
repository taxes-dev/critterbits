#include <string>

#include <critterbits.hpp>

using namespace Critterbits;

int main(int argc, char ** argv) {
    // determine asset path and load configuration
    std::string asset_path = CB_DEFAULT_ASSET_PATH;
    if (argc > 1) {
        asset_path = std::string(argv[1]);
    }
    std::shared_ptr<EngineConfiguration> config = std::make_shared<EngineConfiguration>(asset_path);

    // assign configuration to engine
    Engine::GetInstance().SetConfiguration(std::move(config));

    // run (blocks until window closed)
    return Engine::GetInstance().Run();
}