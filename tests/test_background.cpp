/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test Background Scrolling and Obstacles
*/

#include "Renderer/SFMLRenderer.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include "ECS/ScrollingSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include <iostream>
#include <memory>
#include <chrono>

using namespace RType;

int main() {
    std::cout << "=== R-Type Background & Obstacles Test ===" << std::endl;

    auto renderer = std::make_shared<Renderer::SFMLRenderer>();

    Renderer::WindowConfig windowConfig;
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.title = "R-Type - Background Test";
    windowConfig.fullscreen = false;

    if (!renderer->CreateWindow(windowConfig)) {
        std::cerr << "Failed to create window!" << std::endl;
        return 1;
    }

    ECS::Registry registry;
    ECS::ScrollingSystem scrollingSystem;
    ECS::RenderingSystem renderingSystem(renderer.get());

    std::cout << "Loading textures..." << std::endl;

    auto bgTexture = renderer->LoadTexture("../assets/backgrounds/Cave_one.png");
    if (bgTexture == Renderer::INVALID_TEXTURE_ID) {
        std::cerr << "Failed to load background texture!" << std::endl;
        return 1;
    }
    auto bgSprite = renderer->CreateSprite(bgTexture, {});

    auto rock1Texture = renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_first.png");
    auto rock1Sprite = renderer->CreateSprite(rock1Texture, {});

    auto rock2Texture = renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_two.png");
    auto rock2Sprite = renderer->CreateSprite(rock2Texture, {});

    auto rock3Texture = renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_three.png");
    auto rock3Sprite = renderer->CreateSprite(rock3Texture, {});

    auto rock4Texture = renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_four.png");
    auto rock4Sprite = renderer->CreateSprite(rock4Texture, {});

    auto rock5Texture = renderer->LoadTexture("../assets/backgrounds/obstacles/obstacle_five.png");
    auto rock5Sprite = renderer->CreateSprite(rock5Texture, {});

    std::cout << "Textures loaded!" << std::endl;

    std::cout << "Creating background entities..." << std::endl;

    auto bgSize = renderer->GetTextureSize(bgTexture);
    float scaleX = 1280.0f / bgSize.x;
    float scaleY = 720.0f / bgSize.y;

    for (int i = 0; i < 3; i++) {
        auto bg = registry.CreateEntity();
        registry.AddComponent<ECS::Position>(bg, ECS::Position(i * 1280.0f, 0.0f));

        auto& bgDrawable = registry.AddComponent<ECS::Drawable>(bg, ECS::Drawable(bgSprite, -100));
        bgDrawable.scale = {scaleX, scaleY};

        registry.AddComponent<ECS::Scrollable>(bg, ECS::Scrollable(-200.0f));
    }

    std::cout << "Creating obstacles..." << std::endl;

    auto obs1 = registry.CreateEntity();
    float obs1Width = 1200.0f;
    float obs1Height = 720.0f;
    registry.AddComponent<ECS::Position>(obs1, ECS::Position(1500.0f, 0.0f));

    auto rock1Size = renderer->GetTextureSize(rock1Texture);
    auto& obs1Drawable = registry.AddComponent<ECS::Drawable>(obs1, ECS::Drawable(rock1Sprite, 1));
    obs1Drawable.scale = {obs1Width / rock1Size.x, obs1Height / rock1Size.y};

    auto& obs1MultiCollider = registry.AddComponent<ECS::BoxCollider>(obs1, ECS::BoxCollider());

    registry.AddComponent<ECS::Scrollable>(obs1, ECS::Scrollable(-200.0f));
    registry.AddComponent<ECS::Obstacle>(obs1, ECS::Obstacle(true));

    auto obs2 = registry.CreateEntity();
    float obs2Width = 1200.0f;
    float obs2Height = 720.0f;
    registry.AddComponent<ECS::Position>(obs2, ECS::Position(2700.0f, 720.0f - obs2Height));  // Bottom - espacement 1500px

    auto rock2Size = renderer->GetTextureSize(rock2Texture);
    auto& obs2Drawable = registry.AddComponent<ECS::Drawable>(obs2, ECS::Drawable(rock2Sprite, 1));
    obs2Drawable.scale = {obs2Width / rock2Size.x, obs2Height / rock2Size.y};


    registry.AddComponent<ECS::Scrollable>(obs2, ECS::Scrollable(-200.0f));
    registry.AddComponent<ECS::Obstacle>(obs2, ECS::Obstacle(true));

    auto obs3 = registry.CreateEntity();
    float obs3Width = 1200.0f;
    float obs3Height = 720.0f;
    registry.AddComponent<ECS::Position>(obs3, ECS::Position(3900.0f, 360.0f - obs3Height/2.0f));  // Middle - espacement 1500px

    auto rock3Size = renderer->GetTextureSize(rock3Texture);
    auto& obs3Drawable = registry.AddComponent<ECS::Drawable>(obs3, ECS::Drawable(rock3Sprite, 1));
    obs3Drawable.scale = {obs3Width / rock3Size.x, obs3Height / rock3Size.y};

    registry.AddComponent<ECS::Scrollable>(obs3, ECS::Scrollable(-200.0f));
    registry.AddComponent<ECS::Obstacle>(obs3, ECS::Obstacle(true));

    auto obs4 = registry.CreateEntity();
    float obs4Width = 1200.0f;
    float obs4Height = 720.0f;
    registry.AddComponent<ECS::Position>(obs4, ECS::Position(5100.0f, 0.0f));

    auto rock4Size = renderer->GetTextureSize(rock4Texture);
    auto& obs4Drawable = registry.AddComponent<ECS::Drawable>(obs4, ECS::Drawable(rock4Sprite, 1));
    obs4Drawable.scale = {obs4Width / rock4Size.x, obs4Height / rock4Size.y};


    auto obs5 = registry.CreateEntity();
    float obs5Width = 1200.0f;
    float obs5Height = 720.0f;
    registry.AddComponent<ECS::Position>(obs5, ECS::Position(6300.0f, 720.0f - obs5Height));

    auto rock5Size = renderer->GetTextureSize(rock5Texture);
    auto& obs5Drawable = registry.AddComponent<ECS::Drawable>(obs5, ECS::Drawable(rock5Sprite, 1));
    obs5Drawable.scale = {obs5Width / rock5Size.x, obs5Height / rock5Size.y};


    auto lastTime = std::chrono::high_resolution_clock::now();

    while (renderer->IsWindowOpen()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        if (renderer->IsKeyPressed(Renderer::Key::Escape)) {
            break;
        }

        scrollingSystem.Update(registry, deltaTime);

        auto backgrounds = registry.GetEntitiesWithComponent<ECS::Scrollable>();
        for (auto bg : backgrounds) {
            if (!registry.HasComponent<ECS::Drawable>(bg)) continue;

            auto& drawable = registry.GetComponent<ECS::Drawable>(bg);
            if (drawable.layer == -100) {
                auto& pos = registry.GetComponent<ECS::Position>(bg);
                if (pos.x < -1280.0f) {
                    pos.x += 1280.0f * 3;
                }
            }
        }

        renderer->BeginFrame();
        renderer->Clear({0.0f, 0.0f, 0.0f, 1.0f});
        renderingSystem.Update(registry, deltaTime);
        renderer->EndFrame();
    }

    std::cout << "Test completed!" << std::endl;
    return 0;
}
