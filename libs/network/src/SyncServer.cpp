/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sync Server implementation
*/

#include "SyncServer.hpp"
#include "Serialization/Deserializer.hpp"
#include <iostream>

namespace RType {
namespace Network {

SyncServer::SyncServer(uint16_t port, ECS::Registry& registry)
    : m_registry(registry)
    , m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
    , m_running(true)
{
    m_socket.non_blocking(true);
    std::cout << "[SyncServer] Started on port " << port << std::endl;
}

SyncServer::~SyncServer()
{
    Stop();
}

void SyncServer::AddClient(uint64_t playerHash, uint8_t playerNumber, ::network::Endpoint endpoint)
{
    ClientInfo client;
    client.playerHash = playerHash;
    client.playerNumber = playerNumber;
    client.endpoint = endpoint;
    client.lastPacketTime = std::chrono::steady_clock::now();
    m_clients.push_back(client);
    std::cout << "[SyncServer] Added client hash=" << playerHash << " num=" << (int)playerNumber << std::endl;
}

void SyncServer::Update()
{
    if (!m_running) return;

    m_currentTick++;
    ReceivePackets();

    // Send snapshots every SERVER_TIME_STEP ticks
    if (m_currentTick % SERVER_TIME_STEP == 0) {
        SendSnapshots();
    }
}

void SyncServer::Stop()
{
    m_running = false;
    m_socket.close();
}

void SyncServer::ReceivePackets()
{
    std::array<uint8_t, 4096> buffer;
    asio::ip::udp::endpoint senderEndpoint;

    while (true) {
        asio::error_code ec;
        size_t len = m_socket.receive_from(asio::buffer(buffer), senderEndpoint, 0, ec);

        if (ec == asio::error::would_block) {
            break;  // No more packets
        }

        if (ec) {
            std::cerr << "[SyncServer] Receive error: " << ec.message() << std::endl;
            break;
        }

        if (len > 0) {
            std::vector<uint8_t> data(buffer.begin(), buffer.begin() + len);
            ::network::Endpoint from(senderEndpoint);
            ProcessPacket(data, from);
        }
    }
}

void SyncServer::ProcessPacket(const std::vector<uint8_t>& data, const ::network::Endpoint&)
{
    if (data.size() < 9) return;  // Min size: hash(8) + opcode(1)

    uint64_t senderHash;
    std::memcpy(&senderHash, data.data(), sizeof(uint64_t));

    // Find client
    ClientInfo* client = nullptr;
    for (auto& c : m_clients) {
        if (c.playerHash == senderHash) {
            client = &c;
            client->lastPacketTime = std::chrono::steady_clock::now();
            break;
        }
    }

    if (!client) return;  // Unknown client

    uint8_t opcode = data[8];

    switch (opcode) {
        case 0x02:  // ACK
            HandleAckPacket(data, *client);
            break;
        case 0x03:  // INPUT (future use)
            HandleInputPacket(data, *client);
            break;
        default:
            break;
    }
}

void SyncServer::HandleAckPacket(const std::vector<uint8_t>& data, ClientInfo& client)
{
    if (data.size() < 13) return;  // hash(8) + opcode(1) + tick(4)

    uint32_t ackTick;
    std::memcpy(&ackTick, data.data() + 9, sizeof(uint32_t));
    client.lastAckTick = ackTick;

    // Mark snapshot as ACKed
    size_t idx = ackTick % SNAPSHOT_NBR;
    if (m_snapshots[idx].snapshot.tick == ackTick) {
        size_t clientIdx = &client - &m_clients[0];
        m_snapshots[idx].ackMask |= (1ULL << clientIdx);
    }
}

void SyncServer::HandleInputPacket(const std::vector<uint8_t>& data, ClientInfo& client)
{
    // Future: process player inputs here
    (void)data;
    (void)client;
}

void SyncServer::SendSnapshots()
{
    // Create snapshot from current registry state
    Snapshot currentSnapshot(m_currentTick, m_registry);

    // Store in history
    size_t idx = m_snapshotIndex % SNAPSHOT_NBR;
    m_snapshots[idx].snapshot = currentSnapshot;
    m_snapshots[idx].ackMask = 0;
    m_snapshotIndex++;

    // Send to each client
    for (auto& client : m_clients) {
        if (client.connected) {
            SendSnapshotToClient(client);
        }
    }
}

void SyncServer::SendSnapshotToClient(ClientInfo& client)
{
    size_t clientIdx = &client - &m_clients[0];
    Snapshot& lastAck = FindLastAck(clientIdx);
    Snapshot& current = m_snapshots[(m_snapshotIndex - 1) % SNAPSHOT_NBR].snapshot;

    // Build delta packet
    Engine::Serializer ser;
    ser.serializeTrivial(client.playerHash);         // 8 bytes
    ser.serializeTrivial(static_cast<uint8_t>(0x01)); // Opcode UPDATE
    ser.serializeTrivial(static_cast<uint32_t>(current.tick));  // Current tick
    ser.serializeTrivial(static_cast<uint32_t>(lastAck.tick));  // Previous tick

    // Compute delta
    diffSnapshots(ser, lastAck, current, [](ECS::Entity, uint8_t) { return true; });

    auto packet = ser.finalize();
    SendTo(std::vector<uint8_t>(
        reinterpret_cast<const uint8_t*>(packet.data()),
        reinterpret_cast<const uint8_t*>(packet.data() + packet.size())
    ), client.endpoint);
}

Snapshot& SyncServer::FindLastAck(size_t clientIndex)
{
    // Find most recent ACKed snapshot for this client
    for (size_t i = 0; i < SNAPSHOT_NBR; ++i) {
        size_t idx = (m_snapshotIndex - 1 - i) % SNAPSHOT_NBR;
        if (m_snapshots[idx].snapshot.tick == 0) continue;
        if (m_snapshots[idx].ackMask & (1ULL << clientIndex)) {
            return m_snapshots[idx].snapshot;
        }
    }
    return m_dummySnapshot;  // No ACK yet
}

void SyncServer::SendTo(const std::vector<uint8_t>& data, const ::network::Endpoint& to)
{
    asio::error_code ec;
    m_socket.send_to(asio::buffer(data), to.raw(), 0, ec);

    if (ec) {
        std::cerr << "[SyncServer] Send error: " << ec.message() << std::endl;
    }
}

} // namespace Network
} // namespace RType
