/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include "GameStateMachine.hpp"
#include "MenuState.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include "Audio/SFMLAudio.hpp"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 4242;
    std::string playerName = "Player1";

    if (argc > 1) {
        serverIp = argv[1];
    }
    if (argc > 2) {
        serverPort = static_cast<uint16_t>(std::stoi(argv[2]));
    }
    if (argc > 3) {
        playerName = argv[3];
    }

    std::cout << "=== R-Type Client ===" << std::endl;
    std::cout << "Server IP: " << serverIp << std::endl;
    std::cout << "Server Port: " << serverPort << std::endl;
    std::cout << "Player Name: " << playerName << std::endl;
    std::cout << "=====================" << std::endl;

    auto renderer = std::make_shared<Renderer::SFMLRenderer>();
    auto audio = std::make_shared<Audio::SFMLAudio>();

    Renderer::WindowConfig config;
    config.title = "R-Type - " + playerName;
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
    context.audio = audio;
    context.serverIp = serverIp;
    context.serverPort = serverPort;
    context.playerName = playerName;

    // Configure audio (master volume default 1.0)
    audio->ConfigureDevice(Audio::AudioConfig{});

    RType::Client::GameStateMachine machine;

    machine.PushState(std::make_unique<RType::Client::MenuState>(machine, context));

    while (renderer->IsWindowOpen() && machine.IsRunning()) {
        float dt = renderer->GetDeltaTime();

        renderer->Update(dt);
        if (audio) {
            audio->Update(dt);
        }
        renderer->BeginFrame();

        machine.HandleInput();
        machine.Update(dt);
        machine.Draw();

        renderer->EndFrame();
    }

    std::cout << "Exiting R-Type client..." << std::endl;
    return 0;
}
