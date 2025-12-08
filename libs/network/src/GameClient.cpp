/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameClient implementation
*/

#include "GameClient.hpp"
#include <iostream>
#include <cstring>
#include <random>
#include <thread>

namespace network {

    GameClient::GameClient(const std::string& serverIp, uint16_t serverPort, const PlayerInfo& localPlayer)
        : m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)), m_serverEndpoint(serverIp, serverPort), m_localPlayer(localPlayer) {
        m_socket.non_blocking(true);

        // CRITICAL: Increase socket receive buffer to handle 60 packets/sec
        // Default kernel buffer (~128KB) can overflow if we don't read fast enough
        asio::socket_base::receive_buffer_size option(1024 * 1024); // 1MB buffer
        m_socket.set_option(option);

        m_inputGenerator = [this]() { return GenerateRandomInputs(); };
    }

    GameClient::~GameClient() {
        Stop();
    }

    bool GameClient::ConnectToServer() {
        std::cout << "[Client " << m_localPlayer.name << "] Connecting to "
                  << m_serverEndpoint.address() << ":" << m_serverEndpoint.port() << std::endl;

        HelloPacket hello;
        hello.playerHash = m_localPlayer.hash;
        std::strncpy(hello.playerName, m_localPlayer.name, PLAYER_NAME_SIZE - 1);

        std::vector<uint8_t> packet(sizeof(HelloPacket));
        std::memcpy(packet.data(), &hello, sizeof(HelloPacket));

        m_socket.send_to(asio::buffer(packet), m_serverEndpoint.raw());
        m_packetsSent++;

        auto startTime = std::chrono::steady_clock::now();
        const float TIMEOUT = 5.0f;

        while (!m_connected) {
            ReceivePackets();

            auto elapsed = std::chrono::duration<float>(
                               std::chrono::steady_clock::now() - startTime)
                               .count();

            if (elapsed > TIMEOUT) {
                std::cerr << "[Client " << m_localPlayer.name << "] Connection timeout!" << std::endl;
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[Client " << m_localPlayer.name << "] Connected to server!" << std::endl;
        return true;
    }

    void GameClient::RunHeadless(float duration) {
        if (!m_connected) {
            std::cerr << "[Client " << m_localPlayer.name << "] Not connected!" << std::endl;
            return;
        }

        m_running = true;

        const float INPUT_RATE = 1.0f / 60.0f;
        auto startTime = std::chrono::steady_clock::now();
        auto lastInputTime = std::chrono::steady_clock::now();

        std::cout << "[Client " << m_localPlayer.name << "] Running headless for " << duration << " seconds..." << std::endl;

        while (m_running) {
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - startTime).count();

            if (elapsed >= duration) {
                break;
            }

            ReceivePackets();

            float inputElapsed = std::chrono::duration<float>(now - lastInputTime).count();
            if (inputElapsed >= INPUT_RATE) {
                uint8_t inputs = m_inputGenerator();
                SendInput(inputs);
                lastInputTime = now;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "[Client " << m_localPlayer.name << "] Stopped" << std::endl;
        std::cout << "[Client " << m_localPlayer.name << "] Stats:" << std::endl;
        std::cout << "  - Packets sent: " << m_packetsSent << std::endl;
        std::cout << "  - Packets received: " << m_packetsReceived << std::endl;
        std::cout << "  - Last server tick: " << m_lastServerTick << std::endl;
    }

    void GameClient::Stop() {
        m_running = false;
    }

    void GameClient::SendInput(uint8_t inputs) {
        InputPacket input;
        input.sequence = m_inputSequence++;
        input.playerHash = m_localPlayer.hash;
        input.inputs = inputs;
        input.timestamp = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());

        std::vector<uint8_t> packet(sizeof(InputPacket));
        std::memcpy(packet.data(), &input, sizeof(InputPacket));

        m_socket.send_to(asio::buffer(packet), m_serverEndpoint.raw());
        m_packetsSent++;
    }

    void GameClient::ReceivePackets() {
        // CRITICAL: Read ALL available packets in the socket buffer
        // If we only read 1 packet per frame, we'll lag behind the server!
        int packetsRead = 0;
        while (packetsRead < 100) { // Safety limit to avoid infinite loop
            std::vector<uint8_t> buffer(2048);

            try {
                asio::ip::udp::endpoint rawEndpoint;
                size_t bytes = m_socket.receive_from(asio::buffer(buffer), rawEndpoint);

                if (bytes > 0) {
                    buffer.resize(bytes);
                    HandlePacket(buffer);
                    m_packetsReceived++;
                    packetsRead++;
                }
            } catch (const asio::system_error& e) {
                if (e.code() == asio::error::would_block) {
                    // No more packets available - this is expected
                    break;
                } else {
                    std::cerr << "[Client " << m_localPlayer.name << "] Error receiving: " << e.what() << std::endl;
                    break;
                }
            }
        }

        static int frameCount = 0;
        if (packetsRead > 0 && frameCount++ % 60 == 0) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();
            std::cout << "[CLIENT RECV] t=" << ms << " Read " << packetsRead << " packets in one ReceivePackets() call" << std::endl;
        }
    }

    void GameClient::HandlePacket(const std::vector<uint8_t>& data) {
        if (data.empty())
            return;

        uint8_t type = data[0];

        switch (static_cast<GamePacket>(type)) {
        case GamePacket::WELCOME:
            HandleWelcome(data);
            break;
        case GamePacket::STATE:
            HandleState(data);
            break;
        case GamePacket::PONG:
            HandlePong(data);
            break;
        default:
            break;
        }
    }

    void GameClient::HandleWelcome(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(WelcomePacket))
            return;

        const WelcomePacket* welcome = reinterpret_cast<const WelcomePacket*>(data.data());
        m_lastServerTick = welcome->serverTick;
        m_connected = true;

        std::cout << "[Client " << m_localPlayer.name << "] Received WELCOME (players: "
                  << static_cast<int>(welcome->playersConnected) << ", tick: " << welcome->serverTick << ")" << std::endl;
    }

    void GameClient::HandleState(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(StatePacketHeader))
            return;

        const StatePacketHeader* header = reinterpret_cast<const StatePacketHeader*>(data.data());
        m_lastServerTick = header->tick;
        m_lastScrollOffset = header->scrollOffset;

        size_t offset = sizeof(StatePacketHeader);
        std::vector<EntityState> entities;

        for (uint16_t i = 0; i < header->entityCount; i++) {
            if (offset + sizeof(EntityState) > data.size())
                break;

            EntityState entity;
            std::memcpy(&entity, data.data() + offset, sizeof(EntityState));
            entities.push_back(entity);

            offset += sizeof(EntityState);
        }

        if (m_stateCallback) {
            m_stateCallback(header->tick, entities);
        }
    }

    void GameClient::HandlePong(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(PongPacket))
            return;

        const PongPacket* pong = reinterpret_cast<const PongPacket*>(data.data());

        uint32_t now = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());

        uint32_t rtt = now - pong->timestamp;
        (void)rtt;
    }

    uint8_t GameClient::GenerateRandomInputs() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 31);

        return static_cast<uint8_t>(dis(gen));
    }

}
