/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameClient
*/

#pragma once

#include "Protocol.hpp"
#include "INetworkModule.hpp"
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <functional>

namespace network {

    class GameClient {
    public:
        GameClient(Network::INetworkModule* network, const std::string& serverIp, uint16_t serverPort,
            const PlayerInfo& localPlayer);
        ~GameClient();

        bool ConnectToServer();
        void RunHeadless(float duration);
        void Stop();

        void SetInputGenerator(std::function<uint8_t()> generator) { m_inputGenerator = generator; }
        void SetStateCallback(std::function<void(uint32_t, const std::vector<EntityState>&, const std::vector<InputAck>&)> callback) {
            m_stateCallback = callback;
        }
        void SetLevelCompleteCallback(std::function<void(uint8_t, uint8_t)> callback) {
            m_levelCompleteCallback = callback;
        }

        uint32_t GetLastServerTick() const { return m_lastServerTick; }
        uint32_t GetInputSequence() const { return m_inputSequence; }
        float GetLastScrollOffset() const { return m_lastScrollOffset; }
        uint64_t GetPacketsSent() const { return m_packetsSent; }
        uint64_t GetPacketsReceived() const { return m_packetsReceived; }
        bool IsConnected() const { return m_connected; }

        void SendInput(uint8_t inputs);
        void ReceivePackets();
    private:
        void HandlePacket(const std::vector<uint8_t>& data);
        void HandleWelcome(const std::vector<uint8_t>& data);
        void HandleState(const std::vector<uint8_t>& data);
        void HandleStateDelta(const std::vector<uint8_t>& data);
        void HandlePong(const std::vector<uint8_t>& data);
        void HandleLevelComplete(const std::vector<uint8_t>& data);
        void SendStateAck(uint32_t stateSequence);

        uint8_t GenerateRandomInputs();

        Network::INetworkModule* m_network = nullptr;
        Network::SocketId m_udpSocket = Network::INVALID_SOCKET_ID;
        Network::Endpoint m_serverEndpoint;

        PlayerInfo m_localPlayer;

        uint32_t m_inputSequence = 0;
        uint32_t m_lastServerTick = 0;
        float m_lastScrollOffset = 0.0f;
        std::atomic<bool> m_connected{false};
        std::atomic<bool> m_running{false};

        std::function<uint8_t()> m_inputGenerator;
        std::function<void(uint32_t, const std::vector<EntityState>&, const std::vector<InputAck>&)> m_stateCallback;
        std::function<void(uint8_t, uint8_t)> m_levelCompleteCallback;

        std::atomic<uint64_t> m_packetsSent{0};
        std::atomic<uint64_t> m_packetsReceived{0};

        std::unordered_map<uint32_t, EntityState> m_entityStates;
        uint32_t m_lastReceivedStateSeq = 0;
        uint32_t m_lastAckedStateSeq = 0;
        static constexpr uint32_t STATE_ACK_INTERVAL = 5;
    };

}
