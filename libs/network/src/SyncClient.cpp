/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sync Client implementation
*/

#include "SyncClient.hpp"
#include "Serialization/Deserializer.hpp"
#include <iostream>

namespace RType {
namespace Network {

SyncClient::SyncClient(const std::string& serverAddr, uint16_t serverPort,
                       uint64_t playerHash, uint8_t playerNumber,
                       ECS::Registry& registry)
    : m_registry(registry)
    , m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0))
    , m_playerHash(playerHash)
    , m_playerNumber(playerNumber)
    , m_running(true)
{
    m_socket.non_blocking(true);

    // Resolve server address
    asio::ip::udp::resolver resolver(m_ioContext);
    auto endpoints = resolver.resolve(asio::ip::udp::v4(), serverAddr, std::to_string(serverPort));
    m_serverEndpoint = *endpoints.begin();

    std::cout << "[SyncClient] Connected to " << serverAddr << ":" << serverPort
              << " (hash=" << playerHash << ", num=" << (int)playerNumber << ")" << std::endl;
}

SyncClient::~SyncClient()
{
    Stop();
}

void SyncClient::Update()
{
    if (!m_running) return;

    ReceivePackets();

    // TODO: Send inputs periodically
}

void SyncClient::Stop()
{
    m_running = false;
    m_socket.close();
}

void SyncClient::ReceivePackets()
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
            std::cerr << "[SyncClient] Receive error: " << ec.message() << std::endl;
            break;
        }

        if (len > 0) {
            m_lastPacketTime = std::chrono::steady_clock::now();
            std::vector<uint8_t> data(buffer.begin(), buffer.begin() + len);
            ProcessPacket(data);
        }
    }
}

void SyncClient::ProcessPacket(const std::vector<uint8_t>& data)
{
    if (data.size() < 9) return;  // Min size: hash(8) + opcode(1)

    uint8_t opcode = data[8];

    switch (opcode) {
        case 0x01:  // UPDATE
            HandleUpdatePacket(data);
            break;
        default:
            break;
    }
}

void SyncClient::HandleUpdatePacket(const std::vector<uint8_t>& data)
{
    if (data.size() < 17) return;  // hash(8) + opcode(1) + currentTick(4) + previousTick(4)

    uint32_t currentTick, previousTick;
    std::memcpy(&currentTick, data.data() + 9, sizeof(uint32_t));
    std::memcpy(&previousTick, data.data() + 13, sizeof(uint32_t));

    std::cout << "[SyncClient] Received UPDATE tick=" << currentTick
              << " prev=" << previousTick << " size=" << data.size() << std::endl;

    // Apply delta updates
    if (data.size() > 17) {
        std::vector<std::byte> deltaData(
            reinterpret_cast<const std::byte*>(data.data() + 17),
            reinterpret_cast<const std::byte*>(data.data() + data.size())
        );

        Engine::Deserializer deser(deltaData);

        while (!deser.isFinished()) {
            try {
                uint32_t entityId;
                uint8_t componentId;
                bool updateType;  // 0x01 = add/modify, 0x00 = remove

                deser.deserializeTrivial(entityId);
                deser.deserializeTrivial(componentId);
                deser.deserializeTrivial(updateType);

                if (updateType == 0x01) {
                    // Add or modify component
                    m_registry.ApplyData(entityId, componentId, deser);
                } else {
                    // Remove component (not implemented yet)
                    // TODO: Add RemoveComponent by ID to Registry
                }
            } catch (const std::exception& e) {
                std::cerr << "[SyncClient] Error applying delta: " << e.what() << std::endl;
                break;
            }
        }
    }

    m_lastReceivedTick = currentTick;

    // Send ACK
    SendAck(currentTick);
}

void SyncClient::SendAck(uint32_t tick)
{
    std::vector<uint8_t> packet;
    packet.resize(13);  // hash(8) + opcode(1) + tick(4)

    std::memcpy(packet.data(), &m_playerHash, sizeof(uint64_t));
    packet[8] = 0x02;  // ACK opcode
    std::memcpy(packet.data() + 9, &tick, sizeof(uint32_t));

    SendTo(packet);
}

void SyncClient::SendTo(const std::vector<uint8_t>& data)
{
    asio::error_code ec;
    m_socket.send_to(asio::buffer(data), m_serverEndpoint, 0, ec);

    if (ec) {
        std::cerr << "[SyncClient] Send error: " << ec.message() << std::endl;
    }
}

} // namespace Network
} // namespace RType
