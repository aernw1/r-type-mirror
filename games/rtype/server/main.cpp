/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Server main entry point
*/

#include "RoomManager.hpp"
#include "GameServer.hpp"
#include "AsioNetworkModule.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <optional>
#include <memory>

int main(int argc, char* argv[]) {
    uint16_t port = 4242;
    size_t minPlayers = 2;
    std::string levelPath = "assets/levels/level1.json";

    if (argc > 1)
        port = std::stoi(argv[1]);
    if (argc > 2)
        minPlayers = std::stoi(argv[2]);
    if (argc > 3)
        levelPath = argv[3];

    std::cout << "\n=== Starting R-Type server with room support on port " << port << " ===" << std::endl;

    auto networkModule = std::make_shared<Network::AsioNetworkModule>();
    networkModule->Initialize(nullptr);

    network::RoomManager roomManager(networkModule.get(), port, MAX_ROOMS, minPlayers);

    std::atomic<bool> gameStarted{false};
    std::optional<uint32_t> startedRoomId;
    std::vector<network::PlayerInfo> gamePlayers;

    roomManager.onGameStart([&](uint32_t roomId, const network::RoomManager::Room& room) {
        gamePlayers.clear();
        for (const auto& maybePlayer : room.players) {
            if (maybePlayer) {
                gamePlayers.push_back(*maybePlayer);
            }
        }
        startedRoomId = roomId;
        gameStarted = true;
    });

    while (true) {
        roomManager.update();

        if (gameStarted) {
            std::cout << "Game started in room " << *startedRoomId << " with " << gamePlayers.size() << " players!" << std::endl;

            std::cout << "Waiting 2 seconds for clients to transition..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::cout << "Starting UDP GameServer on port " << port << " with " << gamePlayers.size() << " players..." << std::endl;
            std::cout << "Level: " << levelPath << std::endl;

            network::GameServer gameServer(networkModule.get(), port, gamePlayers, levelPath);
            gameServer.Run();

            std::cout << "\n=== Game ended in room " << *startedRoomId << ". Room available again. ===" << std::endl;

            gameStarted = false;
            startedRoomId.reset();
            gamePlayers.clear();

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
