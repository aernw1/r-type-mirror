#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include "ECS/Component.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include <memory>
#include <chrono>

int main(int, char*[]) {
    using namespace RType;

    Core::Logger::SetLogLevel(Core::LogLevel::Debug);

    auto engine = std::make_unique<Core::Engine>();
    if (!engine->Initialize()) {
        Core::Logger::Critical("Failed to initialize engine");
        return 1;
    }

    auto renderer = std::make_unique<Renderer::SFMLRenderer>();
    auto* rendererPtr = renderer.get();
    engine->RegisterModule(std::move(renderer));

    if (!rendererPtr->Initialize(engine.get())) {
        Core::Logger::Critical("Failed to initialize SFMLRenderer");
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

    auto renderingSystem = std::make_unique<ECS::RenderingSystem>(rendererPtr);
    engine->RegisterSystem(std::move(renderingSystem));

    auto& registry = engine->GetRegistry();

    Core::Logger::Info("Loading sprite assets...");

    auto playerTexture = rendererPtr->LoadTexture("assets/player.png");
    if (playerTexture == Renderer::INVALID_TEXTURE_ID) {
        Core::Logger::Error("Failed to load player texture");
        return 1;
    }
    auto playerSprite = rendererPtr->CreateSprite(playerTexture, Renderer::Rectangle{{0, 0}, {50, 50}});

    auto enemyTexture = rendererPtr->LoadTexture("assets/enemy.png");
    if (enemyTexture == Renderer::INVALID_TEXTURE_ID) {
        Core::Logger::Error("Failed to load enemy texture");
        return 1;
    }
    auto enemySprite = rendererPtr->CreateSprite(enemyTexture, Renderer::Rectangle{{0, 0}, {40, 40}});

    Core::Logger::Info("Creating test entities...");

    auto player = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(player, ECS::Position(640.0f, 360.0f));
    registry.AddComponent<ECS::Drawable>(player, ECS::Drawable(playerSprite, 1));

    auto enemy1 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy1, ECS::Position(200.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy1, ECS::Velocity(100.0f, 50.0f));
    registry.AddComponent<ECS::Drawable>(enemy1, ECS::Drawable(enemySprite, 0));

    auto enemy2 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy2, ECS::Position(640.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy2, ECS::Velocity(-80.0f, 60.0f));
    registry.AddComponent<ECS::Drawable>(enemy2, ECS::Drawable(enemySprite, 0));

    auto enemy3 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy3, ECS::Position(1080.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy3, ECS::Velocity(-120.0f, 40.0f));
    registry.AddComponent<ECS::Drawable>(enemy3, ECS::Drawable(enemySprite, 0));

    Core::Logger::Info("Created {} entities", registry.GetEntityCount());

    auto lastTime = std::chrono::steady_clock::now();

    Core::Logger::Info("Starting render loop...");

    while (rendererPtr->IsWindowOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        rendererPtr->Update(deltaTime);

        // Handle input
        auto& playerPos = registry.GetComponent<ECS::Position>(player);
        float speed = 300.0f;

        if (rendererPtr->IsKeyPressed(Renderer::Key::Left))
            playerPos.x -= speed * deltaTime;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Right))
            playerPos.x += speed * deltaTime;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Up))
            playerPos.y -= speed * deltaTime;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Down))
            playerPos.y += speed * deltaTime;
        if (rendererPtr->IsKeyPressed(Renderer::Key::Escape))
            break;

        // Begin rendering
        rendererPtr->BeginFrame();
        rendererPtr->Clear(Renderer::Color{0.1f, 0.1f, 0.2f, 1.0f});

        // Update all systems (including MovementSystem and RenderingSystem)
        // RenderingSystem will draw sprites automatically
        engine->UpdateSystems(deltaTime);

        rendererPtr->EndFrame();
    }

    Core::Logger::Info("Shutting down...");
    rendererPtr->Shutdown();
    engine->Shutdown();

    return 0;
}
