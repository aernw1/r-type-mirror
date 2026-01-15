#include "Core/Engine.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/InputSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include "BreakoutPhysicsSystem.hpp"
#include "BreakoutRenderSystem.hpp"
#include "BallAccelerationSystem.hpp"
#include <iostream>
#include <memory>
#include <random>

using namespace RType;
using namespace RType::ECS;
using namespace Breakout::ECS;

const Math::Color BRICK_COLORS[] = {
    {1.0f, 0.2f, 0.2f, 1.0f},
    {1.0f, 0.5f, 0.2f, 1.0f},
    {1.0f, 1.0f, 0.2f, 1.0f},
    {0.2f, 1.0f, 0.2f, 1.0f},
    {0.2f, 0.5f, 1.0f, 1.0f},
    {0.5f, 0.2f, 1.0f, 1.0f},
    {0.2f, 1.0f, 1.0f, 1.0f}
};

Entity CreatePaddle(Registry& registry, float x, float y) {
    Entity paddle = registry.CreateEntity();
    registry.AddComponent<Position>(paddle, Position{x, y});
    registry.AddComponent<Velocity>(paddle, Velocity{0.0f, 0.0f});
    registry.AddComponent<Controllable>(paddle, Controllable{400.0f});
    registry.AddComponent<BoxCollider>(paddle, BoxCollider{100.0f, 20.0f});

    Drawable drawable(Renderer::INVALID_SPRITE_ID, 10);
    drawable.tint = {0.8f, 0.8f, 1.0f, 1.0f};
    registry.AddComponent<Drawable>(paddle, std::move(drawable));
    return paddle;
}

Entity CreateBall(Registry& registry, float x, float y) {
    Entity ball = registry.CreateEntity();
    registry.AddComponent<Position>(ball, Position{x, y});

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(-45.0f, 45.0f);

    float angle = angleDist(gen) * (3.14159f / 180.0f);
    float velX = 200.0f * std::sin(angle);
    float velY = 200.0f * std::abs(std::cos(angle));

    registry.AddComponent<Velocity>(ball, Velocity{velX, velY});
    registry.AddComponent<CircleCollider>(ball, CircleCollider{10.0f});

    Drawable drawable(Renderer::INVALID_SPRITE_ID, 12);
    drawable.tint = {1.0f, 1.0f, 1.0f, 1.0f};
    registry.AddComponent<Drawable>(ball, std::move(drawable));
    return ball;
}

Entity CreateBrick(Registry& registry, float x, float y, const Math::Color& color) {
    Entity brick = registry.CreateEntity();
    registry.AddComponent<Position>(brick, Position{x, y});
    registry.AddComponent<Health>(brick, Health{1, 1});
    registry.AddComponent<BoxCollider>(brick, BoxCollider{80.0f, 30.0f});

    Drawable drawable(Renderer::INVALID_SPRITE_ID, 5);
    drawable.tint = color;
    registry.AddComponent<Drawable>(brick, std::move(drawable));
    return brick;
}

void CreateBrickWall(Registry& registry, float startX, float startY, int rows, int cols) {
    const float BRICK_WIDTH = 80.0f;
    const float BRICK_HEIGHT = 30.0f;
    const float BRICK_SPACING = 5.0f;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float x = startX + col * (BRICK_WIDTH + BRICK_SPACING);
            float y = startY + row * (BRICK_HEIGHT + BRICK_SPACING);
            CreateBrick(registry, x, y, BRICK_COLORS[row % 7]);
        }
    }
}

int main(int /*argc*/, char* /*argv*/[]) {
    std::cout << "=== Breakout Game - Engine Reusability Demo ===" << std::endl;

    auto renderer = std::make_shared<Renderer::SFMLRenderer>();
    Renderer::WindowConfig config;
    config.title = "Breakout - Engine Demo";
    config.width = 800;
    config.height = 600;
    config.resizable = false;
    config.fullscreen = false;
    config.targetFramerate = 60;

    if (!renderer->CreateWindow(config)) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    Renderer::Camera2D camera;
    camera.center = Renderer::Vector2(400.0f, 300.0f);
    camera.size = Renderer::Vector2(800.0f, 600.0f);
    renderer->SetCamera(camera);

    Core::EngineConfig engineConfig;
    engineConfig.pluginPath = "./plugins";
    auto engine = std::make_unique<Core::Engine>(engineConfig);

    if (!engine->Initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }

    auto& registry = engine->GetRegistry();

    const float SCREEN_WIDTH = 800.0f;
    const float SCREEN_HEIGHT = 600.0f;
    const int BRICK_COLS = 9;
    const int BRICK_ROWS = 7;
    const float BRICK_WIDTH = 80.0f;
    const float BRICK_SPACING = 5.0f;
    const float WALL_TOTAL_WIDTH = BRICK_COLS * BRICK_WIDTH + (BRICK_COLS - 1) * BRICK_SPACING;
    const float WALL_START_X = (SCREEN_WIDTH - WALL_TOTAL_WIDTH) / 2.0f + BRICK_WIDTH / 2.0f;

    Entity paddle = CreatePaddle(registry, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 80.0f);
    Entity ball = CreateBall(registry, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f);
    CreateBrickWall(registry, WALL_START_X, 30.0f, BRICK_ROWS, BRICK_COLS);

    Entity scoreEntity = registry.CreateEntity();
    registry.AddComponent<Position>(scoreEntity, Position{20.0f, SCREEN_HEIGHT - 40.0f});

    Renderer::FontId font = renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 16);
    if (font == Renderer::INVALID_FONT_ID) {
        font = renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 16);
    }

    Renderer::FontId winFont = renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 80);
    if (winFont == Renderer::INVALID_FONT_ID) {
        winFont = renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 80);
    }
    if (winFont == Renderer::INVALID_FONT_ID) {
        winFont = font;
    }

    if (font != Renderer::INVALID_FONT_ID) {
        TextLabel scoreLabel("SCORE: 0000", font, 16);
        scoreLabel.color = {1.0f, 1.0f, 1.0f, 1.0f};
        registry.AddComponent<TextLabel>(scoreEntity, std::move(scoreLabel));
    }

    Entity winWEntity = NULL_ENTITY;
    Entity winIEntity = NULL_ENTITY;
    Entity winNEntity = NULL_ENTITY;

    engine->RegisterSystem(std::make_unique<InputSystem>(renderer.get()));
    engine->RegisterSystem(std::make_unique<MovementSystem>());
    engine->RegisterSystem(std::make_unique<BallAccelerationSystem>());
    engine->RegisterSystem(std::make_unique<BreakoutPhysicsSystem>());
    engine->RegisterSystem(std::make_unique<RenderingSystem>(renderer.get()));
    engine->RegisterSystem(std::make_unique<BreakoutRenderSystem>(renderer.get()));
    engine->RegisterSystem(std::make_unique<TextRenderingSystem>(renderer.get()));

    int score = 0;
    bool running = true;

    while (renderer->IsWindowOpen() && running) {
        float deltaTime = renderer->GetDeltaTime();
        renderer->Update(deltaTime);
        renderer->BeginFrame();
        renderer->Clear({0.0f, 0.0f, 0.0f, 1.0f});

        if (registry.IsEntityAlive(paddle) && registry.HasComponent<Velocity>(paddle)) {
            registry.GetComponent<Velocity>(paddle).dy = 0.0f;
        }

        engine->UpdateSystems(deltaTime);

        if (registry.IsEntityAlive(paddle) && registry.HasComponent<Position>(paddle) && registry.HasComponent<BoxCollider>(paddle)) {
            auto& paddlePos = registry.GetComponent<Position>(paddle);
            const auto& collider = registry.GetComponent<BoxCollider>(paddle);
            paddlePos.y = SCREEN_HEIGHT - 80.0f;
            float halfWidth = collider.width / 2.0f;
            paddlePos.x = std::max(halfWidth, std::min(SCREEN_WIDTH - halfWidth, paddlePos.x));
        }

        auto brickEntities = registry.GetEntitiesWithComponent<Health>();
        int remainingBricks = 0;
        for (auto entity : brickEntities) {
            if (!registry.HasComponent<Controllable>(entity)) {
                remainingBricks++;
            }
        }

        int newScore = (BRICK_ROWS * BRICK_COLS) - remainingBricks;
        if (newScore != score) {
            score = newScore;
            if (registry.HasComponent<TextLabel>(scoreEntity)) {
                char scoreText[32];
                std::snprintf(scoreText, sizeof(scoreText), "SCORE: %04d", score);
                registry.GetComponent<TextLabel>(scoreEntity).text = scoreText;
            }
        }

        static bool winMessageShown = false;
        if (remainingBricks == 0 && !winMessageShown) {
            std::cout << "You won! All bricks destroyed!" << std::endl;

            if (registry.IsEntityAlive(paddle)) {
                registry.DestroyEntity(paddle);
            }
            if (registry.IsEntityAlive(ball)) {
                registry.DestroyEntity(ball);
            }

            const float WIN_Y = SCREEN_HEIGHT / 2.0f - 50.0f;
            const float LETTER_SPACING = 120.0f;
            const float WIN_START_X = SCREEN_WIDTH / 2.0f - LETTER_SPACING;

            winWEntity = registry.CreateEntity();
            registry.AddComponent<Position>(winWEntity, Position{WIN_START_X, WIN_Y});
            TextLabel wLabel("W", winFont, 80);
            wLabel.color = {1.0f, 0.2f, 0.2f, 1.0f};
            wLabel.centered = true;
            registry.AddComponent<TextLabel>(winWEntity, std::move(wLabel));

            winIEntity = registry.CreateEntity();
            registry.AddComponent<Position>(winIEntity, Position{WIN_START_X + LETTER_SPACING, WIN_Y});
            TextLabel iLabel("I", winFont, 80);
            iLabel.color = {1.0f, 0.5f, 0.2f, 1.0f};
            iLabel.centered = true;
            registry.AddComponent<TextLabel>(winIEntity, std::move(iLabel));

            winNEntity = registry.CreateEntity();
            registry.AddComponent<Position>(winNEntity, Position{WIN_START_X + LETTER_SPACING * 2, WIN_Y});
            TextLabel nLabel("N", winFont, 80);
            nLabel.color = {1.0f, 1.0f, 0.2f, 1.0f};
            nLabel.centered = true;
            registry.AddComponent<TextLabel>(winNEntity, std::move(nLabel));

            winMessageShown = true;
        }

        renderer->EndFrame();

        if (renderer->IsKeyPressed(Renderer::Key::Escape)) {
            running = false;
        }
    }

    std::cout << "Final Score: " << score << std::endl;
    engine->Shutdown();
    return 0;
}
