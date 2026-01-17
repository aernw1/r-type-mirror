/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameClient implementation
*/

#include "GameClient.hpp"
#include "Compression.hpp"
#include <iostream>
#include <cstring>
#include <random>
#include <thread>

namespace network {

    GameClient::GameClient(Network::INetworkModule* network, const std::string& serverIp, uint16_t serverPort,
        const PlayerInfo& localPlayer)
        : m_network(network), m_serverEndpoint(serverIp, serverPort), m_localPlayer(localPlayer) {

        m_udpSocket = m_network->CreateUdpSocket();
        m_network->BindUdp(m_udpSocket, 0);

        m_inputGenerator = [this]() { return GenerateRandomInputs(); };
    }

    GameClient::~GameClient() {
        Stop();
        if (m_network && m_udpSocket != Network::INVALID_SOCKET_ID) {
            m_network->CloseSocket(m_udpSocket);
            m_udpSocket = Network::INVALID_SOCKET_ID;
        }
    }

    bool GameClient::ConnectToServer() {
        std::cout << "[Client " << m_localPlayer.name << "] Connecting to " << m_serverEndpoint.address << ":" << m_serverEndpoint.port << std::endl;

        HelloPacket hello;
        hello.playerHash = m_localPlayer.hash;
        std::strncpy(hello.playerName, m_localPlayer.name, PLAYER_NAME_SIZE - 1);

        std::vector<uint8_t> packet(sizeof(HelloPacket));
        std::memcpy(packet.data(), &hello, sizeof(HelloPacket));

        m_network->SendUdp(m_udpSocket, packet, m_serverEndpoint);
        m_packetsSent++;

        auto startTime = std::chrono::steady_clock::now();
        const float TIMEOUT = 5.0f;

        while (!m_connected) {
            ReceivePackets();

            auto elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

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
        if (m_running && m_connected) {
            std::cout << "[GameClient] Sending DISCONNECT to server..." << std::endl;
            std::vector<uint8_t> disconnectData(1);
            disconnectData[0] = static_cast<uint8_t>(GamePacket::DISCONNECT);

            try {
                m_network->SendUdp(m_udpSocket, disconnectData, m_serverEndpoint);
            } catch (const std::exception& e) {
                std::cerr << "[GameClient] Failed to send DISCONNECT: " << e.what() << std::endl;
            }
        }

        m_running = false;
        m_connected = false;
    }

    void GameClient::SendInput(uint8_t inputs) {
        InputPacket input;
        input.sequence = m_inputSequence++;
        input.playerHash = m_localPlayer.hash;
        input.inputs = inputs;
        input.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

        std::vector<uint8_t> packet(sizeof(InputPacket));
        std::memcpy(packet.data(), &input, sizeof(InputPacket));

        m_network->SendUdp(m_udpSocket, packet, m_serverEndpoint);
        m_packetsSent++;
    }

    void GameClient::ReceivePackets() {
        int packetsRead = 0;
        while (packetsRead < 100) {
            auto packet = m_network->ReceiveUdp(m_udpSocket, 65536);
            if (!packet) {
                break;
            }
            if (!packet->data.empty()) {
                HandlePacket(packet->data);
                m_packetsReceived++;
                packetsRead++;
            }
        }

        static int frameCount = 0;
        if (packetsRead > 0 && frameCount++ % 60 == 0) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
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
        case GamePacket::STATE_DELTA:
            HandleStateDelta(data);
            break;
        case GamePacket::PONG:
            HandlePong(data);
            break;
        case GamePacket::LEVEL_COMPLETE:
            HandleLevelComplete(data);
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

        std::cout << "[Client " << m_localPlayer.name << "] Received WELCOME (players: " << static_cast<int>(welcome->playersConnected) << ", tick: " << welcome->serverTick << ")" << std::endl;
    }

    void GameClient::HandleState(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(StatePacketHeader))
            return;

        const StatePacketHeader* header = reinterpret_cast<const StatePacketHeader*>(data.data());
        m_lastServerTick = header->tick;
        m_lastScrollOffset = header->scrollOffset;
        m_lastReceivedStateSeq = header->stateSequence;

        size_t offset = sizeof(StatePacketHeader);

        std::vector<InputAck> inputAcks;
        for (uint8_t i = 0; i < header->inputAckCount; i++) {
            if (offset + sizeof(InputAck) > data.size())
                break;

            InputAck ack;
            std::memcpy(&ack, data.data() + offset, sizeof(InputAck));
            inputAcks.push_back(ack);
            offset += sizeof(InputAck);
        }

        std::vector<EntityState> entities;
        m_entityStates.clear();

        for (uint16_t i = 0; i < header->entityCount; i++) {
            if (offset + sizeof(EntityState) > data.size())
                break;

            EntityState entity;
            std::memcpy(&entity, data.data() + offset, sizeof(EntityState));
            entities.push_back(entity);
            m_entityStates[entity.entityId] = entity;

            offset += sizeof(EntityState);
        }

        if (m_lastReceivedStateSeq - m_lastAckedStateSeq >= STATE_ACK_INTERVAL) {
            SendStateAck(m_lastReceivedStateSeq);
            m_lastAckedStateSeq = m_lastReceivedStateSeq;
        }

        if (m_stateCallback) {
            m_stateCallback(header->tick, entities, inputAcks);
        }
    }

    void GameClient::HandleStateDelta(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(StateDeltaHeader))
            return;

        const StateDeltaHeader* header = reinterpret_cast<const StateDeltaHeader*>(data.data());
        m_lastServerTick = header->tick;
        m_lastScrollOffset = header->scrollOffset;
        m_lastReceivedStateSeq = header->stateSequence;

        const uint8_t* payloadData = data.data() + sizeof(StateDeltaHeader);
        size_t payloadSize = data.size() - sizeof(StateDeltaHeader);

        std::vector<uint8_t> decompressedPayload;
        if (header->compressionFlags == CompressionFlags::COMPRESSION_LZ4) {
            if (header->uncompressedSize == 0) {
                return;
            }
            decompressedPayload = Compression::DecompressLZ4(payloadData, payloadSize, header->uncompressedSize);
            if (decompressedPayload.empty()) {
                std::cerr << "[CLIENT] LZ4 decompression failed!" << std::endl;
                return;
            }
            payloadData = decompressedPayload.data();
            payloadSize = decompressedPayload.size();
        }

        size_t offset = 0;

        std::vector<InputAck> inputAcks;
        for (uint8_t i = 0; i < header->inputAckCount; i++) {
            if (offset + sizeof(InputAck) > payloadSize)
                break;

            InputAck ack;
            std::memcpy(&ack, payloadData + offset, sizeof(InputAck));
            inputAcks.push_back(ack);
            offset += sizeof(InputAck);
        }

        for (uint16_t i = 0; i < header->destroyedCount; i++) {
            if (offset + sizeof(uint32_t) > payloadSize)
                break;

            uint32_t destroyedId;
            std::memcpy(&destroyedId, payloadData + offset, sizeof(uint32_t));
            m_entityStates.erase(destroyedId);
            offset += sizeof(uint32_t);
        }

        for (uint16_t i = 0; i < header->newEntityCount; i++) {
            if (offset + sizeof(EntityState) > payloadSize)
                break;

            EntityState entity;
            std::memcpy(&entity, payloadData + offset, sizeof(EntityState));
            m_entityStates[entity.entityId] = entity;
            offset += sizeof(EntityState);
        }

        std::vector<DeltaEntityHeader> deltaHeaders;
        for (uint16_t i = 0; i < header->deltaEntityCount; i++) {
            if (offset + sizeof(DeltaEntityHeader) > payloadSize)
                break;

            DeltaEntityHeader deh;
            std::memcpy(&deh, payloadData + offset, sizeof(DeltaEntityHeader));
            deltaHeaders.push_back(deh);
            offset += sizeof(DeltaEntityHeader);
        }

        for (const auto& deh : deltaHeaders) {
            auto it = m_entityStates.find(deh.entityId);
            if (it == m_entityStates.end()) {
                if (deh.deltaFlags & DeltaFlags::DELTA_POSITION) offset += sizeof(float) * 2;
                if (deh.deltaFlags & DeltaFlags::DELTA_VELOCITY) offset += sizeof(float) * 2;
                if (deh.deltaFlags & DeltaFlags::DELTA_HEALTH) offset += sizeof(uint16_t);
                if (deh.deltaFlags & DeltaFlags::DELTA_FLAGS) offset += sizeof(uint8_t);
                if (deh.deltaFlags & DeltaFlags::DELTA_SCORE) offset += sizeof(uint32_t);
                if (deh.deltaFlags & DeltaFlags::DELTA_POWERUP) offset += sizeof(uint8_t) * 2;
                if (deh.deltaFlags & DeltaFlags::DELTA_WEAPON) offset += sizeof(uint8_t) * 2;
                continue;
            }

            EntityState& state = it->second;

            if (deh.deltaFlags & DeltaFlags::DELTA_POSITION) {
                if (offset + sizeof(float) * 2 <= payloadSize) {
                    std::memcpy(&state.x, payloadData + offset, sizeof(float));
                    offset += sizeof(float);
                    std::memcpy(&state.y, payloadData + offset, sizeof(float));
                    offset += sizeof(float);
                }
            }
            if (deh.deltaFlags & DeltaFlags::DELTA_VELOCITY) {
                if (offset + sizeof(float) * 2 <= payloadSize) {
                    std::memcpy(&state.vx, payloadData + offset, sizeof(float));
                    offset += sizeof(float);
                    std::memcpy(&state.vy, payloadData + offset, sizeof(float));
                    offset += sizeof(float);
                }
            }
            if (deh.deltaFlags & DeltaFlags::DELTA_HEALTH) {
                if (offset + sizeof(uint16_t) <= payloadSize) {
                    std::memcpy(&state.health, payloadData + offset, sizeof(uint16_t));
                    offset += sizeof(uint16_t);
                }
            }
            if (deh.deltaFlags & DeltaFlags::DELTA_FLAGS) {
                if (offset + sizeof(uint8_t) <= payloadSize) {
                    state.flags = payloadData[offset++];
                }
            }
            if (deh.deltaFlags & DeltaFlags::DELTA_SCORE) {
                if (offset + sizeof(uint32_t) <= payloadSize) {
                    std::memcpy(&state.score, payloadData + offset, sizeof(uint32_t));
                    offset += sizeof(uint32_t);
                }
            }
            if (deh.deltaFlags & DeltaFlags::DELTA_POWERUP) {
                if (offset + sizeof(uint8_t) * 2 <= payloadSize) {
                    state.powerUpFlags = payloadData[offset++];
                    state.speedMultiplier = payloadData[offset++];
                }
            }
            if (deh.deltaFlags & DeltaFlags::DELTA_WEAPON) {
                if (offset + sizeof(uint8_t) * 2 <= payloadSize) {
                    state.weaponType = payloadData[offset++];
                    state.fireRate = payloadData[offset++];
                }
            }
        }

        if (m_lastReceivedStateSeq - m_lastAckedStateSeq >= STATE_ACK_INTERVAL) {
            SendStateAck(m_lastReceivedStateSeq);
            m_lastAckedStateSeq = m_lastReceivedStateSeq;
        }

        std::vector<EntityState> entities;
        entities.reserve(m_entityStates.size());
        for (const auto& [id, state] : m_entityStates) {
            entities.push_back(state);
        }

        if (m_stateCallback) {
            m_stateCallback(header->tick, entities, inputAcks);
        }
    }

    void GameClient::SendStateAck(uint32_t stateSequence) {
        StateAckPacket ack;
        ack.playerHash = m_localPlayer.hash;
        ack.lastReceivedSeq = stateSequence;

        std::vector<uint8_t> packet(sizeof(StateAckPacket));
        std::memcpy(packet.data(), &ack, sizeof(StateAckPacket));

        m_network->SendUdp(m_udpSocket, packet, m_serverEndpoint);
        m_packetsSent++;
    }

    void GameClient::HandlePong(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(PongPacket))
            return;

        const PongPacket* pong = reinterpret_cast<const PongPacket*>(data.data());

        uint32_t now = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

        uint32_t rtt = now - pong->timestamp;
        (void)rtt;
    }

    void GameClient::HandleLevelComplete(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(LevelCompletePacket))
            return;

        const LevelCompletePacket* packet = reinterpret_cast<const LevelCompletePacket*>(data.data());

        std::cout << "[GameClient] Level " << static_cast<int>(packet->completedLevel)
                  << " complete! Next level: " << static_cast<int>(packet->nextLevel) << std::endl;

        if (m_levelCompleteCallback) {
            m_levelCompleteCallback(packet->completedLevel, packet->nextLevel);
        }
    }

    uint8_t GameClient::GenerateRandomInputs() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 31);

        return static_cast<uint8_t>(dis(gen));
    }

}
