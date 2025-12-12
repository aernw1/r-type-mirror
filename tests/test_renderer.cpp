#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Core/Platform.hpp"
#include "Renderer/IRenderer.hpp"
#include "ECS/Component.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/CollisionDetectionSystem.hpp"
#include <memory>
#include <chrono>
#include <vector>
#include <cmath>

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
        Core::Logger::Critical("Make sure to run from build/ directory: ./bin/test_renderer");
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
    windowConfig.title = "R-Type - ECS & Network Test";
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.fullscreen = false;
    windowConfig.resizable = true;
    windowConfig.targetFramerate = 60;

    if (!rendererPtr->CreateWindow(windowConfig)) {
        Core::Logger::Critical("Failed to create window");
        return 1;
    }
    auto movementSystem = std::make_unique<ECS::MovementSystem>();
    engine->RegisterSystem(std::move(movementSystem));

    auto collisionSystem = std::make_unique<ECS::CollisionDetectionSystem>();
    engine->RegisterSystem(std::move(collisionSystem));

    auto renderingSystem = std::make_unique<ECS::RenderingSystem>(rendererPtr);
    engine->RegisterSystem(std::move(renderingSystem));

    auto& registry = engine->GetRegistry();

    Core::Logger::Info("Loading sprite assets...");

    auto playerTexture = rendererPtr->LoadTexture("assets/spaceships/player_red.png");
    if (playerTexture == Renderer::INVALID_TEXTURE_ID) {
        Core::Logger::Error("Failed to load player texture");
        return 1;
    }
    Core::Logger::Info("Player texture loaded: ID={}", playerTexture);
    auto playerSprite = rendererPtr->CreateSprite(playerTexture, Renderer::Rectangle{{0, 0}, {50, 50}});
    Core::Logger::Info("Player sprite created: ID={}", playerSprite);

    auto enemyTexture = rendererPtr->LoadTexture("assets/spaceships/nave2.png");
    if (enemyTexture == Renderer::INVALID_TEXTURE_ID) {
        Core::Logger::Error("Failed to load enemy texture");
        return 1;
    }
    Core::Logger::Info("Enemy texture loaded: ID={}", enemyTexture);
    auto enemySprite = rendererPtr->CreateSprite(enemyTexture, Renderer::Rectangle{{0, 0}, {40, 40}});
    Core::Logger::Info("Enemy sprite created: ID={}", enemySprite);

    Core::Logger::Info("Creating test entities...");

    auto player = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(player, ECS::Position(640.0f, 360.0f));
    registry.AddComponent<ECS::Drawable>(player, ECS::Drawable(playerSprite, 1));
    registry.AddComponent<ECS::BoxCollider>(player, ECS::BoxCollider(50.0f, 50.0f));

    auto enemy1 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy1, ECS::Position(200.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy1, ECS::Velocity(100.0f, 50.0f));
    registry.AddComponent<ECS::Drawable>(enemy1, ECS::Drawable(enemySprite, 0));
    registry.AddComponent<ECS::BoxCollider>(enemy1, ECS::BoxCollider(40.0f, 40.0f));

    auto enemy2 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy2, ECS::Position(640.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy2, ECS::Velocity(-80.0f, 60.0f));
    registry.AddComponent<ECS::Drawable>(enemy2, ECS::Drawable(enemySprite, 0));
    registry.AddComponent<ECS::BoxCollider>(enemy2, ECS::BoxCollider(40.0f, 40.0f));

    auto enemy3 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy3, ECS::Position(1080.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy3, ECS::Velocity(-120.0f, 40.0f));
    registry.AddComponent<ECS::Drawable>(enemy3, ECS::Drawable(enemySprite, 0));
    registry.AddComponent<ECS::BoxCollider>(enemy3, ECS::BoxCollider(40.0f, 40.0f));

    Core::Logger::Info("Created {} entities", registry.GetEntityCount());

    std::vector<ECS::Entity> enemies = {enemy1, enemy2, enemy3};

    auto lastTime = std::chrono::steady_clock::now();

    Core::Logger::Info("Starting render loop...");
    Core::Logger::Info("Move the player with arrow keys - collisions will be logged!");

    while (rendererPtr->IsWindowOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        rendererPtr->Update(deltaTime);

        auto& playerPos = registry.GetComponent<ECS::Position>(player);
        constexpr float speed = 300.0f;

        const float movement = speed * deltaTime;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Left))
            playerPos.x -= movement;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Right))
            playerPos.x += movement;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Up))
            playerPos.y -= movement;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Down))
            playerPos.y += movement;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Escape))
            break;

        rendererPtr->BeginFrame();
        rendererPtr->Clear(Renderer::Color{0.1f, 0.1f, 0.2f, 1.0f});
        engine->UpdateSystems(deltaTime);

        bool playerColliding = false;
        for (auto enemy : enemies) {
            if (!registry.IsEntityAlive(enemy)) continue;

            // Simple AABB collision check for test purposes
            auto& playerPos = registry.GetComponent<ECS::Position>(player);
            auto& enemyPos = registry.GetComponent<ECS::Position>(enemy);
            bool colliding = false;
            if (registry.HasComponent<ECS::BoxCollider>(player) && registry.HasComponent<ECS::BoxCollider>(enemy)) {
                auto& playerBox = registry.GetComponent<ECS::BoxCollider>(player);
                auto& enemyBox = registry.GetComponent<ECS::BoxCollider>(enemy);
                float dx = std::abs(playerPos.x - enemyPos.x);
                float dy = std::abs(playerPos.y - enemyPos.y);
                colliding = (dx < (playerBox.width + enemyBox.width) / 2.0f &&
                            dy < (playerBox.height + enemyBox.height) / 2.0f);
            }
            if (colliding) {
                auto& enemyPos = registry.GetComponent<ECS::Position>(enemy);
                Core::Logger::Warning("COLLISION! Player hit enemy at ({}, {})",
                                     enemyPos.x, enemyPos.y);

                auto& enemyDrawable = registry.GetComponent<ECS::Drawable>(enemy);
                enemyDrawable.tint = Renderer::Color{1.0f, 0.0f, 0.0f, 1.0f};

                playerColliding = true;
            } else {
                auto& enemyDrawable = registry.GetComponent<ECS::Drawable>(enemy);
                enemyDrawable.tint = Renderer::Color{1.0f, 1.0f, 1.0f, 1.0f};
            }
        }

        auto& playerDrawable = registry.GetComponent<ECS::Drawable>(player);
        if (playerColliding) {
            playerDrawable.tint = Renderer::Color{1.0f, 0.5f, 0.0f, 1.0f};
        } else {
            playerDrawable.tint = Renderer::Color{1.0f, 1.0f, 1.0f, 1.0f};
        }

        auto& playerCol = registry.GetComponent<ECS::BoxCollider>(player);
        rendererPtr->DrawRectangle(
            Renderer::Rectangle{{playerPos.x, playerPos.y}, {playerCol.width, playerCol.height}},
            Renderer::Color{0.0f, 1.0f, 0.0f, 0.5f}
        );

        for (auto enemy : enemies) {
            if (!registry.IsEntityAlive(enemy)) continue;
            auto& enemyPos = registry.GetComponent<ECS::Position>(enemy);
            auto& enemyCol = registry.GetComponent<ECS::BoxCollider>(enemy);
            rendererPtr->DrawRectangle(
                Renderer::Rectangle{{enemyPos.x, enemyPos.y}, {enemyCol.width, enemyCol.height}},
                Renderer::Color{1.0f, 0.0f, 0.0f, 0.5f}
            );
        }

        rendererPtr->EndFrame();
    }

    Core::Logger::Info("Shutting down...");
    engine->Shutdown();

    return 0;
}
