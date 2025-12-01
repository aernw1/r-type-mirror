#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Core/Platform.hpp"
#include "Renderer/IRenderer.hpp"
#include "ECS/Component.hpp"
#include "ECS/InputSystem.hpp"
#include "ECS/PlayerSystem.hpp"
#include "ECS/PlayerFactory.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include <memory>
#include <chrono>

int main(int, char*[]) {
    using namespace RType;

    Core::Logger::SetLogLevel(Core::LogLevel::Debug);

    auto engine = std::make_unique<Core::Engine>();
    Core::IModule* module = nullptr;

    module = engine->LoadPlugin("lib/" + Core::Platform::GetPluginPath("SFMLRenderer"));
    if (!module) {
        module = engine->LoadPlugin(Core::Platform::GetPluginPathFromBin("SFMLRenderer"));
    }

    if (!module) {
        Core::Logger::Critical("Failed to load SFMLRenderer plugin");
        Core::Logger::Critical("Make sure to run from build/ directory: ./bin/test_player_system");
        return 1;
    }

    auto* rendererPtr = dynamic_cast<Renderer::IRenderer*>(module);
    if (!rendererPtr) {
        Core::Logger::Critical("Failed to cast module to IRenderer");
        return 1;
    }

    if (!engine->Initialize()) {
        Core::Logger::Critical("Failed to initialize engine");
        return 1;
    }

    Renderer::WindowConfig windowConfig;
    windowConfig.title = "R-Type - PlayerSystem Test";
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.fullscreen = false;
    windowConfig.resizable = true;
    windowConfig.targetFramerate = 60;

    if (!rendererPtr->CreateWindow(windowConfig)) {
        Core::Logger::Critical("Failed to create window");
        return 1;
    }

    Core::Logger::Info("Registering systems...");

    auto inputSystem = std::make_unique<ECS::InputSystem>(rendererPtr);
    engine->RegisterSystem(std::move(inputSystem));

    auto playerSystem = std::make_unique<ECS::PlayerSystem>(rendererPtr);
    engine->RegisterSystem(std::move(playerSystem));

    auto movementSystem = std::make_unique<ECS::MovementSystem>();
    engine->RegisterSystem(std::move(movementSystem));

    auto renderingSystem = std::make_unique<ECS::RenderingSystem>(rendererPtr);
    engine->RegisterSystem(std::move(renderingSystem));

    auto& registry = engine->GetRegistry();

    Core::Logger::Info("Creating players...");

    for (uint8_t i = 1; i <= 4; ++i) {
        uint64_t playerHash = static_cast<uint64_t>(i) * 1000;
        ECS::PlayerFactory::CreatePlayer(registry, i, playerHash, 100.0f, 200.0f, rendererPtr);
    }

    Core::Logger::Info("Created {} entities", registry.GetEntityCount());
    Core::Logger::Info("Controls: Arrow keys to move, ESC to quit");

    auto lastTime = std::chrono::steady_clock::now();

    Core::Logger::Info("Starting render loop...");

    while (rendererPtr->IsWindowOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        rendererPtr->Update(deltaTime);

        if (rendererPtr->IsKeyPressed(Renderer::Key::Escape)) {
            break;
        }

        rendererPtr->BeginFrame();
        rendererPtr->Clear(Renderer::Color{0.1f, 0.1f, 0.2f, 1.0f});
        engine->UpdateSystems(deltaTime);
        rendererPtr->EndFrame();
    }

    Core::Logger::Info("Shutting down...");
    engine->Shutdown();

    return 0;
}

