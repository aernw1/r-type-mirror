#include <iostream>
#include "../include/Core/Engine.hpp"

using namespace RType;

int main() {
    Core::Logger::Info("R-Type Engine starting...");

    Core::EngineConfig config;
    config.pluginPath = "./plugins";

    auto engine = std::make_unique<Core::Engine>(config);

    if (!engine->Initialize()) {
        Core::Logger::Error("Failed to initialize engine");
        return 1;
    }
    Core::Logger::Info("Engine initialized with {} modules", engine->GetAllModules().size());

    auto& registry = engine->GetRegistry();
    Core::Logger::Info("Registry has {} entities", registry.GetEntityCount());

    engine->Shutdown();

    Core::Logger::Info("Engine stopped");
    return 0;
}
