/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sync Server for ECS-based networking
*/

#pragma once

#include "Snapshot.hpp"
#include "ECS/Registry.hpp"
#include "Protocol.hpp"
#include "Endpoint.hpp"
#include <asio.hpp>
#include <array>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace RType {
namespace Network {

constexpr size_t SNAPSHOT_NBR = 32;
constexpr size_t SERVER_TIME_STEP = 6;  // Send snapshots every 6 ticks (60 tick/s → 10 snapshot/s)

struct ClientInfo {
    uint64_t playerHash;
    uint8_t playerNumber;
    ::network::Endpoint endpoint;
    std::chrono::steady_clock::time_point lastPacketTime;
    size_t lastAckTick = 0;
    bool connected = true;
};

class SyncServer {
public:
    SyncServer(uint16_t port, ECS::Registry& registry);
    ~SyncServer();

    void AddClient(uint64_t playerHash, uint8_t playerNumber, ::network::Endpoint endpoint);
    void Update();  // Call this every tick (60 Hz)
    void Stop();

    size_t GetCurrentTick() const { return m_currentTick; }

private:
    struct SnapshotHistory {
        Snapshot snapshot;
        uint64_t ackMask = 0;  // Bitmask of clients who ACKed
    };

    void ReceivePackets();
    void ProcessPacket(const std::vector<uint8_t>& data, const ::network::Endpoint& from);
    void HandleInputPacket(const std::vector<uint8_t>& data, ClientInfo& client);
    void HandleAckPacket(const std::vector<uint8_t>& data, ClientInfo& client);

    void SendSnapshots();
    void SendSnapshotToClient(ClientInfo& client);
    Snapshot& FindLastAck(size_t clientIndex);

    void SendTo(const std::vector<uint8_t>& data, const ::network::Endpoint& to);

    ECS::Registry& m_registry;
    asio::io_context m_ioContext;
    asio::ip::udp::socket m_socket;

    std::array<SnapshotHistory, SNAPSHOT_NBR> m_snapshots;
    size_t m_snapshotIndex = 0;
    size_t m_currentTick = 0;

    std::vector<ClientInfo> m_clients;
    Snapshot m_dummySnapshot;  // Used when no ACK received yet

    bool m_running = false;
};

} // namespace Network
} // namespace RType
