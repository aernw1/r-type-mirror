#include "GameStateMachine.hpp"
#include "Menu/MainMenu.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    RType::Client::GameContext context;
    context.window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(1280, 720), "R-Type Client"
    );
    context.window->setFramerateLimit(60);

    RType::Client::GameStateMachine machine;
    
    machine.PushState(std::make_unique<RType::Client::MainMenu>(machine, context));

    sf::Clock clock;
    while (context.window->isOpen() && machine.IsRunning()) {
        float dt = clock.restart().asSeconds();
        
        machine.HandleInput();
        machine.Update(dt);
        machine.Draw();
    }

    return 0;
}

/*
#include "LobbyClient.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
   std::string addr = "127.0.0.1";
   uint16_t port = 4242;
   std::string name = "Player";

   if (argc > 1)
       addr = argv[1];
   if (argc > 2)
       port = std::stoi(argv[2]);
   if (argc > 3)
       name = argv[3];

   std::cout << "Connecting to " << addr << ":" << port << " as " << name << std::endl;

   network::LobbyClient client(addr, port);
   client.connect(name);

   while (!client.isConnected()) {
       client.update();
       std::this_thread::sleep_for(std::chrono::milliseconds(16));
   }

   std::cout << "Press Enter to ready up..." << std::endl;
   std::cin.get();
   client.ready();

   std::cout << "Press Enter to start game..." << std::endl;
   std::cin.get();
   client.requestStart();

   while (!client.isGameStarted()) {
       client.update();
       std::this_thread::sleep_for(std::chrono::milliseconds(16));
   }

   std::cout << "Game started!" << std::endl;

   return 0;
}
*/
