#include "LobbyServer.hpp"
#include "LobbyClient.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

int main() {
    std::cout << "=== Lobby Test ===" << std::endl;

    std::thread serverThread([]() {
        network::LobbyServer server(4242, 4);
        std::cout << "[Server] Started on port 4242" << std::endl;

        while (!server.isGameStarted()) {
            server.update();
            std::this_thread::sleep_for(16ms);
        }
        std::cout << "[Server] Game started!" << std::endl;
    });

    std::this_thread::sleep_for(100ms);

    std::thread client1Thread([]() {
        network::LobbyClient client("127.0.0.1", 4242);
        client.connect("Alice");

        while (!client.isConnected()) {
            client.update();
            std::this_thread::sleep_for(16ms);
        }
        std::cout << "[Alice] Connected!" << std::endl;

        std::this_thread::sleep_for(200ms);
        client.ready();
        std::cout << "[Alice] Ready!" << std::endl;

        while (!client.isGameStarted()) {
            client.update();
            std::this_thread::sleep_for(16ms);
        }
        std::cout << "[Alice] Game started!" << std::endl;
    });

    std::this_thread::sleep_for(100ms);

    std::thread client2Thread([]() {
        network::LobbyClient client("127.0.0.1", 4242);
        client.connect("Bob");

        while (!client.isConnected()) {
            client.update();
            std::this_thread::sleep_for(16ms);
        }
        std::cout << "[Bob] Connected!" << std::endl;

        std::this_thread::sleep_for(200ms);
        client.ready();
        std::cout << "[Bob] Ready!" << std::endl;

        std::this_thread::sleep_for(200ms);
        client.requestStart();
        std::cout << "[Bob] Requested start!" << std::endl;

        while (!client.isGameStarted()) {
            client.update();
            std::this_thread::sleep_for(16ms);
        }
        std::cout << "[Bob] Game started!" << std::endl;
    });

    serverThread.join();
    client1Thread.join();
    client2Thread.join();

    std::cout << "=== Test Complete ===" << std::endl;
    return 0;
}
