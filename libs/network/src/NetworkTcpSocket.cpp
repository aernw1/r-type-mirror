/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** NetworkTcpSocket implementation
*/

#include "NetworkTcpSocket.hpp"
#include <iostream>

namespace network {

    NetworkTcpSocket::NetworkTcpSocket(Network::INetworkModule* network, Network::SocketId socketId)
        : m_network(network), m_socketId(socketId), m_connected(true) {
    }

    NetworkTcpSocket::NetworkTcpSocket(Network::INetworkModule* network, const std::string& address, uint16_t port)
        : m_network(network), m_socketId(Network::INVALID_SOCKET_ID), m_connected(false) {

        m_socketId = m_network->CreateTcpSocket();
        Network::Endpoint endpoint{address, port};

        if (m_network->ConnectTcp(m_socketId, endpoint)) {
            m_connected = true;
        } else {
            std::cerr << "[NetworkTcpSocket] Failed to connect to " << address << ":" << port << std::endl;
        }
    }

    NetworkTcpSocket::~NetworkTcpSocket() {
        if (m_socketId != Network::INVALID_SOCKET_ID && m_network) {
            m_network->CloseSocket(m_socketId);
        }
    }

    NetworkTcpSocket::NetworkTcpSocket(NetworkTcpSocket&& other) noexcept
        : m_network(other.m_network), m_socketId(other.m_socketId), m_connected(other.m_connected) {
        other.m_socketId = Network::INVALID_SOCKET_ID;
        other.m_connected = false;
    }

    NetworkTcpSocket& NetworkTcpSocket::operator=(NetworkTcpSocket&& other) noexcept {
        if (this != &other) {
            if (m_socketId != Network::INVALID_SOCKET_ID && m_network) {
                m_network->CloseSocket(m_socketId);
            }
            m_network = other.m_network;
            m_socketId = other.m_socketId;
            m_connected = other.m_connected;
            other.m_socketId = Network::INVALID_SOCKET_ID;
            other.m_connected = false;
        }
        return *this;
    }

    void NetworkTcpSocket::send(const std::vector<uint8_t>& data) {
        if (!m_network || m_socketId == Network::INVALID_SOCKET_ID) {
            return;
        }

        if (!m_network->SendTcp(m_socketId, data)) {
            auto error = m_network->GetLastError();
            if (error == Network::SocketError::Disconnected) {
                m_connected = false;
            }
        }
    }

    std::optional<std::vector<uint8_t>> NetworkTcpSocket::receive() {
        if (!m_network || m_socketId == Network::INVALID_SOCKET_ID) {
            return std::nullopt;
        }

        auto result = m_network->ReceiveTcp(m_socketId);
        if (!result) {
            auto error = m_network->GetLastError();
            if (error == Network::SocketError::Disconnected) {
                m_connected = false;
            }
        }
        return result;
    }

    bool NetworkTcpSocket::isConnected() const {
        return m_connected && m_socketId != Network::INVALID_SOCKET_ID;
    }

    void NetworkTcpSocket::disconnect() {
        if (m_network && m_socketId != Network::INVALID_SOCKET_ID) {
            m_network->CloseSocket(m_socketId);
            m_socketId = Network::INVALID_SOCKET_ID;
        }
        m_connected = false;
    }

    // NetworkTcpServer implementation

    NetworkTcpServer::NetworkTcpServer(Network::INetworkModule* network, uint16_t port)
        : m_network(network), m_serverSocket(Network::INVALID_SOCKET_ID), m_port(port) {

        m_serverSocket = m_network->CreateTcpSocket();
        m_network->BindTcp(m_serverSocket, port);
        m_network->ListenTcp(m_serverSocket);
    }

    NetworkTcpServer::~NetworkTcpServer() {
        if (m_network && m_serverSocket != Network::INVALID_SOCKET_ID) {
            m_network->CloseSocket(m_serverSocket);
        }
    }

    std::optional<NetworkTcpSocket> NetworkTcpServer::accept() {
        if (!m_network || m_serverSocket == Network::INVALID_SOCKET_ID) {
            return std::nullopt;
        }

        Network::Endpoint clientEndpoint;
        auto clientSocketId = m_network->AcceptTcp(m_serverSocket, clientEndpoint);

        if (clientSocketId) {
            return NetworkTcpSocket(m_network, *clientSocketId);
        }

        return std::nullopt;
    }

}
