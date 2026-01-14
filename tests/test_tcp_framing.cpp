/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TCP framing/accept smoke test for INetworkModule (AsioNetworkModule)
*/

#include "AsioNetworkModule.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

static bool waitUntil(std::function<bool()> predicate, std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < timeout) {
        if (predicate()) {
            return true;
        }
        std::this_thread::sleep_for(2ms);
    }
    return false;
}

int main() {
    std::cout << "=== TCP Framing Test ===" << std::endl;

    auto networkModule = std::make_shared<Network::AsioNetworkModule>();
    if (!networkModule->Initialize(nullptr)) {
        std::cerr << "Failed to initialize network module" << std::endl;
        return 1;
    }

    // Start server
    Network::SocketId serverSocket = networkModule->CreateTcpSocket();
    if (!networkModule->BindTcp(serverSocket, 0)) {
        std::cerr << "BindTcp failed" << std::endl;
        return 1;
    }
    if (!networkModule->ListenTcp(serverSocket)) {
        std::cerr << "ListenTcp failed" << std::endl;
        return 1;
    }

    Network::SocketInfo serverInfo = networkModule->GetSocketInfo(serverSocket);
    std::uint16_t port = serverInfo.localEndpoint.port;
    if (port == 0) {
        std::cerr << "Server did not get a port" << std::endl;
        return 1;
    }

    // Non-blocking accept should return no client before connect.
    Network::Endpoint ignoredEndpoint;
    if (networkModule->AcceptTcp(serverSocket, ignoredEndpoint).has_value()) {
        std::cerr << "AcceptTcp unexpectedly returned a client before connect" << std::endl;
        return 1;
    }

    std::vector<std::uint8_t> msg1{1, 2, 3, 4};
    std::vector<std::uint8_t> msg2{9, 8, 7};

    std::thread clientThread([&]() {
        Network::SocketId clientSocket = networkModule->CreateTcpSocket();
        if (!networkModule->ConnectTcp(clientSocket, Network::Endpoint{"127.0.0.1", port})) {
            std::cerr << "[Client] ConnectTcp failed" << std::endl;
            return;
        }
        // Send back-to-back to increase chance both frames arrive in one read.
        networkModule->SendTcp(clientSocket, msg1);
        networkModule->SendTcp(clientSocket, msg2);
        std::this_thread::sleep_for(50ms);
        networkModule->CloseSocket(clientSocket);
    });

    Network::SocketId acceptedClient = Network::INVALID_SOCKET_ID;
    Network::Endpoint clientEp;
    bool accepted = waitUntil([&]() {
        auto client = networkModule->AcceptTcp(serverSocket, clientEp);
        if (client) {
            acceptedClient = *client;
            return true;
        }
        return false;
    }, 2000ms);

    if (!accepted || acceptedClient == Network::INVALID_SOCKET_ID) {
        std::cerr << "Server did not accept client" << std::endl;
        clientThread.join();
        return 1;
    }

    std::vector<std::uint8_t> got1;
    std::vector<std::uint8_t> got2;

    bool gotBoth = waitUntil([&]() {
        if (got1.empty()) {
            auto r = networkModule->ReceiveTcp(acceptedClient, 2048);
            if (r) {
                got1 = *r;
            }
        }
        if (!got1.empty() && got2.empty()) {
            auto r = networkModule->ReceiveTcp(acceptedClient, 2048);
            if (r) {
                got2 = *r;
            }
        }
        return !got1.empty() && !got2.empty();
    }, 2000ms);

    networkModule->CloseSocket(acceptedClient);
    networkModule->CloseSocket(serverSocket);
    clientThread.join();

    if (!gotBoth) {
        std::cerr << "Did not receive both framed messages" << std::endl;
        return 1;
    }

    if (got1 != msg1) {
        std::cerr << "First message mismatch (size=" << got1.size() << ")" << std::endl;
        return 1;
    }
    if (got2 != msg2) {
        std::cerr << "Second message mismatch (size=" << got2.size() << ")" << std::endl;
        return 1;
    }

    std::cout << "OK: accepted client + received 2 framed messages" << std::endl;
    return 0;
}

