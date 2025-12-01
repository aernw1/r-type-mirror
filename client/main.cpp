#include "GameStateMachine.hpp"
#include "Menu/MainMenu.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include <iostream>
#include <memory>

int main(int, char*[]) {
    auto renderer = std::make_shared<Renderer::SFMLRenderer>();
    Renderer::WindowConfig config;
    config.title = "R-Type Client";
    config.width = 1280;
    config.height = 720;
    config.targetFramerate = 60;
    
    if (!renderer->CreateWindow(config)) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    RType::Client::GameContext context;
    context.renderer = renderer;
    context.registry = std::make_shared<RType::ECS::Registry>();

    RType::Client::GameStateMachine machine;
    
    machine.PushState(std::make_unique<RType::Client::MainMenu>(machine, context));

    sf::Clock clock;
    while (renderer->IsWindowOpen() && machine.IsRunning()) {
        float dt = clock.restart().asSeconds();
        
        renderer->Update(dt);
        
        renderer->BeginFrame();
        
        machine.HandleInput();
        machine.Update(dt);
        machine.Draw();
        
        renderer->EndFrame();
    }

    return 0;
}

