#include "LobbyServer.hpp"
#include "GameServer.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    uint16_t port = 4242;
    size_t minPlayers = 2;

    if (argc > 1)
        port = std::stoi(argv[1]);
    if (argc > 2)
        minPlayers = std::stoi(argv[2]);

    std::cout << "Starting lobby server on port " << port << std::endl;

    network::LobbyServer server(port, 4, minPlayers);

    size_t lastPlayerCount = 0;

    while (!server.isGameStarted()) {
        server.update();

        if (server.playerCount() != lastPlayerCount) {
            server.printStatus();
            lastPlayerCount = server.playerCount();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "Game started with " << server.playerCount() << " players!" << std::endl;

    std::vector<network::PlayerInfo> players;
    for (const auto& maybePlayer : server.getPlayers()) {
        if (maybePlayer) {
            players.push_back(*maybePlayer);
        }
    }

    std::cout << "Waiting 2 seconds for clients to transition..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Starting UDP GameServer on port " << port << " with " << players.size() << " players..." << std::endl;

    network::GameServer gameServer(port, players);
    gameServer.Run();

    return 0;
}
