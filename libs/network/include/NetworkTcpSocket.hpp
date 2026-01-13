/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** NetworkTcpSocket - Adapter for INetworkModule TCP sockets
*/

#pragma once

#include "INetworkModule.hpp"
#include <vector>
#include <cstdint>
#include <optional>

namespace network {

    // Adapter class that provides TcpSocket-like interface using INetworkModule
    class NetworkTcpSocket {
    public:
        NetworkTcpSocket(Network::INetworkModule* network, Network::SocketId socketId);
        NetworkTcpSocket(Network::INetworkModule* network, const std::string& address, uint16_t port);
        ~NetworkTcpSocket();

        NetworkTcpSocket(const NetworkTcpSocket&) = delete;
        NetworkTcpSocket& operator=(const NetworkTcpSocket&) = delete;
        NetworkTcpSocket(NetworkTcpSocket&& other) noexcept;
        NetworkTcpSocket& operator=(NetworkTcpSocket&& other) noexcept;

        void send(const std::vector<uint8_t>& data);
        std::optional<std::vector<uint8_t>> receive();

        bool isConnected() const;
        void disconnect();

        Network::SocketId getSocketId() const { return m_socketId; }

    private:
        Network::INetworkModule* m_network;
        Network::SocketId m_socketId;
        bool m_connected;
    };

    // Adapter class for TCP server using INetworkModule
    class NetworkTcpServer {
    public:
        explicit NetworkTcpServer(Network::INetworkModule* network, uint16_t port);
        ~NetworkTcpServer();

        NetworkTcpServer(const NetworkTcpServer&) = delete;
        NetworkTcpServer& operator=(const NetworkTcpServer&) = delete;

        std::optional<NetworkTcpSocket> accept();
        uint16_t port() const { return m_port; }

    private:
        Network::INetworkModule* m_network;
        Network::SocketId m_serverSocket;
        uint16_t m_port;
    };

}
