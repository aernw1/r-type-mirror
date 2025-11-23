#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include "ECS/Component.hpp"
#include "ECS/MovementSystem.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Config.hpp>
#include <memory>

#if SFML_VERSION_MAJOR >= 3
#define RTYPE_SFML_3
#endif

int main(int argc, char* argv[]) {
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

    auto& registry = engine->GetRegistry();

    Core::Logger::Info("Creating test entities...");

    auto player = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(player, ECS::Position(640.0f, 360.0f));

    auto enemy1 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy1, ECS::Position(200.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy1, ECS::Velocity(100.0f, 50.0f));

    auto enemy2 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy2, ECS::Position(640.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy2, ECS::Velocity(-80.0f, 60.0f));

    auto enemy3 = registry.CreateEntity();
    registry.AddComponent<ECS::Position>(enemy3, ECS::Position(1080.0f, 100.0f));
    registry.AddComponent<ECS::Velocity>(enemy3, ECS::Velocity(-120.0f, 40.0f));

    Core::Logger::Info("Created {} entities", registry.GetEntityCount());

    sf::Clock clock;

    Core::Logger::Info("Starting render loop...");

    while (rendererPtr->IsWindowOpen()) {
        float deltaTime = clock.restart().asSeconds();

        rendererPtr->Update(deltaTime);
        engine->UpdateSystems(deltaTime);

        auto& playerPos = registry.GetComponent<ECS::Position>(player);
        float speed = 300.0f;

#ifdef RTYPE_SFML_3
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            playerPos.x -= speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            playerPos.x += speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            playerPos.y -= speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            playerPos.y += speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            break;
#else
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            playerPos.x -= speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            playerPos.x += speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            playerPos.y -= speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            playerPos.y += speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            break;
#endif

        rendererPtr->BeginFrame();
        rendererPtr->Clear(Renderer::Color{0.1f, 0.1f, 0.2f, 1.0f});

        Renderer::Rectangle playerRect{{playerPos.x - 25, playerPos.y - 25}, {50, 50}};
        rendererPtr->DrawRectangle(playerRect, Renderer::Color{0.2f, 0.8f, 0.2f, 1.0f});

        auto& enemy1Pos = registry.GetComponent<ECS::Position>(enemy1);
        Renderer::Rectangle rect1{{enemy1Pos.x - 20, enemy1Pos.y - 20}, {40, 40}};
        rendererPtr->DrawRectangle(rect1, Renderer::Color{0.8f, 0.2f, 0.2f, 1.0f});

        auto& enemy2Pos = registry.GetComponent<ECS::Position>(enemy2);
        Renderer::Rectangle rect2{{enemy2Pos.x - 20, enemy2Pos.y - 20}, {40, 40}};
        rendererPtr->DrawRectangle(rect2, Renderer::Color{0.8f, 0.2f, 0.2f, 1.0f});

        auto& enemy3Pos = registry.GetComponent<ECS::Position>(enemy3);
        Renderer::Rectangle rect3{{enemy3Pos.x - 20, enemy3Pos.y - 20}, {40, 40}};
        rendererPtr->DrawRectangle(rect3, Renderer::Color{0.8f, 0.2f, 0.2f, 1.0f});
        rendererPtr->EndFrame();
    }

    Core::Logger::Info("Shutting down...");
    rendererPtr->Shutdown();
    engine->Shutdown();

    return 0;
}
