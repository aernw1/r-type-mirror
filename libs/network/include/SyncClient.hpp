/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sync Client for ECS-based networking
*/

#pragma once

#include "Snapshot.hpp"
#include "ECS/Registry.hpp"
#include <asio.hpp>
#include <string>
#include <vector>
#include <chrono>

namespace RType {
namespace Network {

class SyncClient {
public:
    SyncClient(const std::string& serverAddr, uint16_t serverPort,
               uint64_t playerHash, uint8_t playerNumber,
               ECS::Registry& registry);
    ~SyncClient();

    void Update();  // Call every frame
    void Stop();

    uint64_t GetPlayerHash() const { return m_playerHash; }
    uint8_t GetPlayerNumber() const { return m_playerNumber; }

private:
    void ReceivePackets();
    void ProcessPacket(const std::vector<uint8_t>& data);
    void HandleUpdatePacket(const std::vector<uint8_t>& data);

    void SendAck(uint32_t tick);
    void SendTo(const std::vector<uint8_t>& data);

    ECS::Registry& m_registry;
    asio::io_context m_ioContext;
    asio::ip::udp::socket m_socket;
    asio::ip::udp::endpoint m_serverEndpoint;

    uint64_t m_playerHash;
    uint8_t m_playerNumber;

    size_t m_lastReceivedTick = 0;
    std::chrono::steady_clock::time_point m_lastPacketTime;

    bool m_running = false;
};

} // namespace Network
} // namespace RType
