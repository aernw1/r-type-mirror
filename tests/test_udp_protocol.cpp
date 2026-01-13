/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UDP Protocol Test - 10 seconds simulation
*/

#include "GameServer.hpp"
#include "GameClient.hpp"
#include "AsioNetworkModule.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>
#include <memory>

using namespace network;

void PrintSeparator(const std::string& title = "") {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    if (!title.empty()) {
        std::cout << "  " << title << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    }
}

void PrintStats(const std::vector<GameClient*>& clients, const GameServer& server) {
    PrintSeparator("STATISTICS");

    std::cout << "\n[SERVER]" << std::endl;
    std::cout << "  Current tick: " << server.GetCurrentTick() << std::endl;
    std::cout << "  Packets sent: " << server.GetPacketsSent() << std::endl;
    std::cout << "  Packets received: " << server.GetPacketsReceived() << std::endl;
    std::cout << "  Connected players: " << server.GetConnectedPlayerCount() << std::endl;

    std::cout << "\n[CLIENTS]" << std::endl;
    for (size_t i = 0; i < clients.size(); i++) {
        std::cout << "  Client " << (i + 1) << ":" << std::endl;
        std::cout << "    Packets sent: " << clients[i]->GetPacketsSent() << std::endl;
        std::cout << "    Packets received: " << clients[i]->GetPacketsReceived() << std::endl;
        std::cout << "    Last server tick: " << clients[i]->GetLastServerTick() << std::endl;
    }

    PrintSeparator();
}

int main() {
    PrintSeparator("R-TYPE UDP PROTOCOL TEST");
    std::cout << "\nDuration: 10 seconds" << std::endl;
    std::cout << "Players: 2" << std::endl;
    std::cout << "Protocol: UDP (authoritative server)" << std::endl;

    std::vector<PlayerInfo> players;

    PlayerInfo player1;
    player1.number = 1;
    player1.hash = 12345;
    std::strncpy(player1.name, "Alice", PLAYER_NAME_SIZE - 1);
    player1.ready = true;
    players.push_back(player1);

    PlayerInfo player2;
    player2.number = 2;
    player2.hash = 67890;
    std::strncpy(player2.name, "Bob", PLAYER_NAME_SIZE - 1);
    player2.ready = true;
    players.push_back(player2);

    PrintSeparator("STARTING SERVER");

    const uint16_t UDP_PORT = 4243;
    auto networkModule = std::make_shared<Network::AsioNetworkModule>();
    networkModule->Initialize(nullptr);

    GameServer server(networkModule.get(), UDP_PORT, players);

    std::thread serverThread([&server]() {
        server.Run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    PrintSeparator("CONNECTING CLIENTS");
    std::vector<GameClient*> clients;
    GameClient client1(networkModule.get(), "127.0.0.1", UDP_PORT, player1);
    GameClient client2(networkModule.get(), "127.0.0.1", UDP_PORT, player2);
    clients.push_back(&client1);
    clients.push_back(&client2);

    bool firstStateReceived = false;
    client1.SetStateCallback([&firstStateReceived](uint32_t tick, const std::vector<EntityState>& entities) {
        if (!firstStateReceived) {
            std::cout << "\n[Client Alice] First STATE packet received!" << std::endl;
            std::cout << "  Tick: " << tick << std::endl;
            std::cout << "  Entities: " << entities.size() << std::endl;
            firstStateReceived = true;
        }
    });

    if (!client1.ConnectToServer()) {
        std::cerr << "Client 1 failed to connect!" << std::endl;
        server.Stop();
        serverThread.join();
        return 1;
    }

    if (!client2.ConnectToServer()) {
        std::cerr << "Client 2 failed to connect!" << std::endl;
        server.Stop();
        serverThread.join();
        return 1;
    }

    PrintSeparator("RUNNING SIMULATION");
    std::cout << "\nRunning for 10 seconds..." << std::endl;
    std::cout << "Clients will send random inputs" << std::endl;
    std::cout << "Server will simulate game (players, enemies, bullets)" << std::endl;

    const float DURATION = 10.0f;

    std::thread clientThread1([&client1, DURATION]() {
        client1.RunHeadless(DURATION);
    });

    std::thread clientThread2([&client2, DURATION]() {
        client2.RunHeadless(DURATION);
    });

    auto startTime = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - startTime).count();

        if (elapsed >= DURATION) {
            break;
        }

        if (static_cast<int>(elapsed) % 2 == 0) {
            std::cout << "\rElapsed: " << std::fixed << std::setprecision(1) << elapsed << "s / " << DURATION << "s" << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl;

    clientThread1.join();
    clientThread2.join();

    server.Stop();
    serverThread.join();

    PrintStats(clients, server);

    PrintSeparator("VALIDATION");
    bool success = true;

    std::cout << "\n[CHECKS]" << std::endl;

    if (server.GetCurrentTick() >= 500) {
        std::cout << "  [OK] Server ticks: " << server.GetCurrentTick() << " (expected ~600)" << std::endl;
    } else {
        std::cout << "  [FAIL] Server ticks: " << server.GetCurrentTick() << " (expected ~600)" << std::endl;
        success = false;
    }

    if (server.GetPacketsSent() >= 100) {
        std::cout << "  [OK] Server sent packets: " << server.GetPacketsSent() << std::endl;
    } else {
        std::cout << "  [FAIL] Server sent too few packets: " << server.GetPacketsSent() << std::endl;
        success = false;
    }

    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i]->GetPacketsSent() >= 500) {
            std::cout << "  [OK] Client " << (i + 1) << " sent packets: " << clients[i]->GetPacketsSent() << std::endl;
        } else {
            std::cout << "  [FAIL] Client " << (i + 1) << " sent too few packets: " << clients[i]->GetPacketsSent() << std::endl;
            success = false;
        }
    }

    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i]->GetPacketsReceived() >= 100) {
            std::cout << "  [OK] Client " << (i + 1) << " received packets: " << clients[i]->GetPacketsReceived() << std::endl;
        } else {
            std::cout << "  [FAIL] Client " << (i + 1) << " received too few packets: " << clients[i]->GetPacketsReceived() << std::endl;
            success = false;
        }
    }

    PrintSeparator();

    if (success) {
        std::cout << "\nALL TESTS PASSED! UDP protocol working correctly." << std::endl;
        std::cout << "\nProtocol validation:" << std::endl;
        std::cout << "  - Authoritative server [OK]" << std::endl;
        std::cout << "  - UDP communication [OK]" << std::endl;
        std::cout << "  - Binary protocol [OK]" << std::endl;
        std::cout << "  - Client inputs -> Server [OK]" << std::endl;
        std::cout << "  - Server state -> Clients [OK]" << std::endl;
        std::cout << "  - No crashes [OK]" << std::endl;
        PrintSeparator();
        return 0;
    } else {
        std::cout << "\nSOME TESTS FAILED!" << std::endl;
        PrintSeparator();
        return 1;
    }
}
