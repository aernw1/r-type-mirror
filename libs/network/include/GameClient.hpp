/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameClient
*/

#pragma once

#include "Protocol.hpp"
#include "Endpoint.hpp"
#include <asio.hpp>
#include <vector>
#include <chrono>
#include <atomic>
#include <functional>

namespace network {

    class GameClient {
    public:
        GameClient(const std::string& serverIp, uint16_t serverPort, const PlayerInfo& localPlayer);
        ~GameClient();

        bool ConnectToServer();
        void RunHeadless(float duration);
        void Stop();

        void SetInputGenerator(std::function<uint8_t()> generator) { m_inputGenerator = generator; }
        void SetStateCallback(std::function<void(uint32_t, const std::vector<EntityState>&)> callback) {
            m_stateCallback = callback;
        }
        void SetLevelCompleteCallback(std::function<void(uint8_t, uint8_t)> callback) {
            m_levelCompleteCallback = callback;
        }

        uint32_t GetLastServerTick() const { return m_lastServerTick; }
        float GetLastScrollOffset() const { return m_lastScrollOffset; }
        uint64_t GetPacketsSent() const { return m_packetsSent; }
        uint64_t GetPacketsReceived() const { return m_packetsReceived; }

        // Network communication (called by game loop)
        void SendInput(uint8_t inputs);
        void ReceivePackets();
    private:
        void HandlePacket(const std::vector<uint8_t>& data);
        void HandleWelcome(const std::vector<uint8_t>& data);
        void HandleState(const std::vector<uint8_t>& data);
        void HandlePong(const std::vector<uint8_t>& data);
        void HandleLevelComplete(const std::vector<uint8_t>& data);

        uint8_t GenerateRandomInputs();

        asio::io_context m_ioContext;
        asio::ip::udp::socket m_socket;
        Endpoint m_serverEndpoint;

        PlayerInfo m_localPlayer;

        uint32_t m_inputSequence = 0;
        uint32_t m_lastServerTick = 0;
        float m_lastScrollOffset = 0.0f;
        std::atomic<bool> m_connected{false};
        std::atomic<bool> m_running{false};

        std::function<uint8_t()> m_inputGenerator;
        std::function<void(uint32_t, const std::vector<EntityState>&)> m_stateCallback;
        std::function<void(uint8_t, uint8_t)> m_levelCompleteCallback; // (completedLevel, nextLevel)

        std::atomic<uint64_t> m_packetsSent{0};
        std::atomic<uint64_t> m_packetsReceived{0};
    };

}
