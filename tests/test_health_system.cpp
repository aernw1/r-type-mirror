/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test Health System
*/

#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Core/Platform.hpp"
#include "Renderer/IRenderer.hpp"
#include "ECS/Component.hpp"
#include "ECS/InputSystem.hpp"
#include "PlayerSystem.hpp"
#include "PlayerFactory.hpp"
#include "EnemySystem.hpp"
#include "EnemyFactory.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/CollisionDetectionSystem.hpp"
#include "ECS/HealthSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "ECS/Components/TextLabel.hpp"
#include <memory>
#include <chrono>
#include <sstream>

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
        Core::Logger::Critical("Make sure to run from build/ directory: ./bin/test_health_system");
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
    windowConfig.title = "R-Type - HealthSystem Test";
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

    auto enemySystem = std::make_unique<ECS::EnemySystem>(rendererPtr);
    engine->RegisterSystem(std::move(enemySystem));

    auto movementSystem = std::make_unique<ECS::MovementSystem>();
    engine->RegisterSystem(std::move(movementSystem));

    auto collisionSystem = std::make_unique<ECS::CollisionDetectionSystem>();
    engine->RegisterSystem(std::move(collisionSystem));

    auto healthSystem = std::make_unique<ECS::HealthSystem>();
    engine->RegisterSystem(std::move(healthSystem));

    auto renderingSystem = std::make_unique<ECS::RenderingSystem>(rendererPtr);
    engine->RegisterSystem(std::move(renderingSystem));

    auto textRenderingSystem = std::make_unique<ECS::TextRenderingSystem>(rendererPtr);
    engine->RegisterSystem(std::move(textRenderingSystem));

    auto& registry = engine->GetRegistry();

    Core::Logger::Info("Creating player...");
    uint64_t playerHash = 1000;
    ECS::Entity player = ECS::PlayerFactory::CreatePlayer(registry, 1, playerHash, 100.0f, 360.0f, rendererPtr);

    Core::Logger::Info("Creating test enemies...");
    ECS::EnemyFactory::CreateEnemy(registry, ECS::EnemyType::BASIC, 800.0f, 300.0f, rendererPtr);
    ECS::EnemyFactory::CreateEnemy(registry, ECS::EnemyType::FAST, 900.0f, 400.0f, rendererPtr);
    ECS::EnemyFactory::CreateEnemy(registry, ECS::EnemyType::TANK, 1000.0f, 500.0f, rendererPtr);

    Core::Logger::Info("Creating health display...");
    Renderer::FontId fontId = rendererPtr->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 20);
    if (fontId == Renderer::INVALID_FONT_ID) {
        Core::Logger::Warning("Failed to load font, health display will not be shown");
    } else {
        ECS::Entity healthDisplay = registry.CreateEntity();
        registry.AddComponent<ECS::Position>(healthDisplay, ECS::Position(20.0f, 20.0f));
        registry.AddComponent<ECS::TextLabel>(healthDisplay, ECS::TextLabel("Health: 100/100", fontId));
        auto& healthLabel = registry.GetComponent<ECS::TextLabel>(healthDisplay);
        healthLabel.color = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
    }

    Core::Logger::Info("Created {} entities", registry.GetEntityCount());
    Core::Logger::Info("Controls: Arrow keys to move player, ESC to quit");
    Core::Logger::Info("Test: Move player into enemies to test health system");
    Core::Logger::Info("Enemies will damage player and be destroyed on collision");

    auto healthDisplayEntities = registry.GetEntitiesWithComponent<ECS::TextLabel>();
    ECS::Entity healthDisplay = ECS::NULL_ENTITY;
    for (ECS::Entity entity : healthDisplayEntities) {
        if (registry.HasComponent<ECS::TextLabel>(entity)) {
            const auto& label = registry.GetComponent<ECS::TextLabel>(entity);
            if (label.text.find("Health:") != std::string::npos) {
                healthDisplay = entity;
                break;
            }
        }
    }

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

        if (healthDisplay != ECS::NULL_ENTITY && registry.IsEntityAlive(player) &&
            registry.HasComponent<ECS::Health>(player) &&
            registry.HasComponent<ECS::TextLabel>(healthDisplay)) {
            const auto& health = registry.GetComponent<ECS::Health>(player);
            auto& healthLabel = registry.GetComponent<ECS::TextLabel>(healthDisplay);

            std::ostringstream oss;
            oss << "Health: " << health.current << "/" << health.max;
            healthLabel.text = oss.str();

            if (health.current <= 30) {
                healthLabel.color = Math::Color(1.0f, 0.0f, 0.0f, 1.0f);
            } else if (health.current <= 50) {
                healthLabel.color = Math::Color(1.0f, 0.5f, 0.0f, 1.0f);
            } else {
                healthLabel.color = Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
            }
        } else if (healthDisplay != ECS::NULL_ENTITY && !registry.IsEntityAlive(player) && registry.HasComponent<ECS::TextLabel>(healthDisplay)) {
            auto& healthLabel = registry.GetComponent<ECS::TextLabel>(healthDisplay);
            healthLabel.text = "Health: 0/100 - PLAYER DEAD";
            healthLabel.color = Math::Color(1.0f, 0.0f, 0.0f, 1.0f);
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
