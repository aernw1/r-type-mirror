/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "GameStateMachine.hpp"
#include "MenuState.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include <iostream>
#include <memory>

int main(int, char*[]) {
    auto renderer = std::make_shared<Renderer::SFMLRenderer>();

    Renderer::WindowConfig config;
    config.title = "R-Type";
    config.width = 1280;
    config.height = 720;
    config.resizable = true;
    config.fullscreen = false;
    config.targetFramerate = 60;

    if (!renderer->CreateWindow(config)) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    RType::Client::GameContext context;
    context.renderer = renderer;

    RType::Client::GameStateMachine machine;

    machine.PushState(std::make_unique<RType::Client::MenuState>(machine, context));

    while (renderer->IsWindowOpen() && machine.IsRunning()) {
        float dt = renderer->GetDeltaTime();

        renderer->Update(dt);
        renderer->BeginFrame();

        machine.HandleInput();
        machine.Update(dt);
        machine.Draw();

        renderer->EndFrame();
    }

    std::cout << "Exiting R-Type client..." << std::endl;
    return 0;
}
