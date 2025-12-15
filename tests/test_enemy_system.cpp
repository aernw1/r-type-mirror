/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test Enemy System
*/

#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Core/Platform.hpp"
#include "Renderer/IRenderer.hpp"
#include "ECS/Component.hpp"
#include "ECS/EnemySystem.hpp"
#include "ECS/EnemyFactory.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include <memory>
#include <chrono>

using namespace RType;

int main(int, char*[]) {

    Core::Logger::SetLogLevel(Core::LogLevel::Debug);

    auto engine = std::make_unique<Core::Engine>();
    Core::IModule* module = nullptr;

    module = engine->LoadPlugin("lib/" + Core::Platform::GetPluginPath("SFMLRenderer"));
    if (!module) {
        module = engine->LoadPlugin(Core::Platform::GetPluginPathFromBin("SFMLRenderer"));
    }

    if (!module) {
        Core::Logger::Critical("Failed to load SFMLRenderer plugin");
        Core::Logger::Critical("Make sure to run from build/ directory: ./bin/test_enemy_system");
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
    windowConfig.title = "R-Type - EnemySystem Test";
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

    auto enemySystem = std::make_unique<ECS::EnemySystem>(rendererPtr);
    engine->RegisterSystem(std::move(enemySystem));

    auto movementSystem = std::make_unique<ECS::MovementSystem>();
    engine->RegisterSystem(std::move(movementSystem));

    auto renderingSystem = std::make_unique<ECS::RenderingSystem>(rendererPtr);
    engine->RegisterSystem(std::move(renderingSystem));

    Core::Logger::Info("EnemySystem will spawn enemies automatically every 3 seconds");
    Core::Logger::Info("Controls: ESC to quit");

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

