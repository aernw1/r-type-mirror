/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameServer implementation
*/

#include "GameServer.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <random>

namespace network {

    GameServer::GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers)
        : m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), m_expectedPlayers(expectedPlayers), m_lastSpawnTime(std::chrono::steady_clock::now()) {

        // Increase send buffer to handle broadcasting to multiple clients at 60Hz
        asio::socket_base::send_buffer_size sendOption(1024 * 1024); // 1MB
        m_socket.set_option(sendOption);

        std::cout << "GameServer started on UDP port " << port << std::endl;
        std::cout << "Waiting for " << expectedPlayers.size() << " players..." << std::endl;
    }

    GameServer::~GameServer() {
        Stop();
    }

    void GameServer::Run() {
        m_running = true;

        WaitForAllPlayers();

        float startX = 100.0f;
        float startY = 300.0f;
        size_t playerIndex = 0;
        for (const auto& [hash, player] : m_connectedPlayers) {
            SpawnPlayer(hash, startX + playerIndex * 100.0f, startY);
            playerIndex++;
        }

        std::cout << "All players connected! Starting game loop..." << std::endl;

        const float TICK_RATE = 1.0f / 60.0f;

        auto lastTick = std::chrono::steady_clock::now();

        while (m_running) {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - lastTick).count();
            lastTick = now;

            ProcessIncomingPackets();

            UpdateGameLogic(TICK_RATE);
            m_currentTick++;

            // Send snapshots EVERY tick for instant responsiveness
            SendStateSnapshots();

            auto elapsed = std::chrono::steady_clock::now() - now;
            auto sleepTime = std::chrono::duration<float>(TICK_RATE) - elapsed;
            if (sleepTime.count() > 0) {
                std::this_thread::sleep_for(sleepTime);
            }
        }

        std::cout << "GameServer stopped" << std::endl;
    }

    void GameServer::Stop() {
        m_running = false;
    }

    void GameServer::WaitForAllPlayers() {
        m_socket.non_blocking(true);

        while (m_connectedPlayers.size() < m_expectedPlayers.size()) {
            std::vector<uint8_t> buffer(1024);
            asio::ip::udp::endpoint rawEndpoint;

            try {
                size_t bytes = m_socket.receive_from(asio::buffer(buffer), rawEndpoint);
                if (bytes > 0) {
                    buffer.resize(bytes);
                    Endpoint clientEndpoint(rawEndpoint);
                    HandleHello(buffer, clientEndpoint);
                }
            } catch (const asio::system_error& e) {
                if (e.code() != asio::error::would_block) {
                    std::cerr << "Error receiving: " << e.what() << std::endl;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "All players connected!" << std::endl;
    }

    void GameServer::ProcessIncomingPackets() {
        std::vector<uint8_t> buffer(1024);
        asio::ip::udp::endpoint rawEndpoint;

        try {
            size_t bytes = m_socket.receive_from(asio::buffer(buffer), rawEndpoint);
            if (bytes > 0) {
                buffer.resize(bytes);
                Endpoint clientEndpoint(rawEndpoint);
                HandlePacket(buffer, clientEndpoint);
                m_packetsReceived++;
            }
        } catch (const asio::system_error& e) {
            if (e.code() != asio::error::would_block) {
                std::cerr << "Error receiving: " << e.what() << std::endl;
            }
        }
    }

    void GameServer::HandlePacket(const std::vector<uint8_t>& data, const Endpoint& from) {
        if (data.empty())
            return;

        uint8_t type = data[0];

        switch (static_cast<GamePacket>(type)) {
        case GamePacket::HELLO:
            HandleHello(data, from);
            break;
        case GamePacket::INPUT:
            HandleInput(data, from);
            break;
        case GamePacket::PING:
            HandlePing(data, from);
            break;
        default:
            break;
        }
    }

    void GameServer::HandleHello(const std::vector<uint8_t>& data, const Endpoint& from) {
        if (data.size() < sizeof(HelloPacket))
            return;

        const HelloPacket* hello = reinterpret_cast<const HelloPacket*>(data.data());

        auto it = std::find_if(m_expectedPlayers.begin(), m_expectedPlayers.end(),
                               [hello](const PlayerInfo& p) { return p.hash == hello->playerHash; });

        if (it != m_expectedPlayers.end()) {
            if (m_connectedPlayers.find(hello->playerHash) != m_connectedPlayers.end()) {
                return;
            }

            ConnectedPlayer player;
            player.info = *it;
            player.endpoint = from;
            player.lastPingTime = std::chrono::steady_clock::now();
            player.alive = true;

            m_connectedPlayers[hello->playerHash] = player;

            WelcomePacket welcome;
            welcome.playersConnected = static_cast<uint8_t>(m_connectedPlayers.size());
            welcome.serverTick = m_currentTick;

            std::vector<uint8_t> response(sizeof(WelcomePacket));
            std::memcpy(response.data(), &welcome, sizeof(WelcomePacket));

            SendTo(response, from);

            std::cout << "Player " << it->name << " connected ("
                      << m_connectedPlayers.size() << "/" << m_expectedPlayers.size() << ")" << std::endl;
        }
    }

    void GameServer::HandleInput(const std::vector<uint8_t>& data, const Endpoint& /*from*/) {
        if (data.size() < sizeof(InputPacket))
            return;

        const InputPacket* input = reinterpret_cast<const InputPacket*>(data.data());

        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        auto it = m_connectedPlayers.find(input->playerHash);
        if (it == m_connectedPlayers.end())
            return;

        static int recvLog = 0;
        if (input->inputs != 0 && recvLog++ % 10 == 0) {
            std::cout << "[SERVER RECV INPUT] t=" << ms << " from=" << it->second.info.name
                      << " inputs=" << (int)input->inputs << " seq=" << input->sequence << std::endl;
        }

        it->second.lastInputSequence = input->sequence;
        it->second.lastPingTime = now;

        auto playerEntity = std::find_if(m_entities.begin(), m_entities.end(),
                                         [hash = input->playerHash](const GameEntity& e) {
                                             return e.type == EntityType::PLAYER && e.ownerHash == hash;
                                         });

        if (playerEntity == m_entities.end())
            return;

        const float SPEED = 300.0f;

        playerEntity->vx = 0.0f;
        playerEntity->vy = 0.0f;

        if (input->inputs & InputFlags::UP)
            playerEntity->vy = -SPEED;
        if (input->inputs & InputFlags::DOWN)
            playerEntity->vy = SPEED;
        if (input->inputs & InputFlags::LEFT)
            playerEntity->vx = -SPEED;
        if (input->inputs & InputFlags::RIGHT)
            playerEntity->vx = SPEED;

        if (input->inputs & InputFlags::SHOOT) {
            SpawnBullet(input->playerHash, playerEntity->x + 50.0f, playerEntity->y + 20.0f);
        }
    }

    void GameServer::HandlePing(const std::vector<uint8_t>& data, const Endpoint& from) {
        if (data.size() < sizeof(PingPacket))
            return;

        const PingPacket* ping = reinterpret_cast<const PingPacket*>(data.data());

        PongPacket pong;
        pong.timestamp = ping->timestamp;

        std::vector<uint8_t> response(sizeof(PongPacket));
        std::memcpy(response.data(), &pong, sizeof(PongPacket));

        SendTo(response, from);
    }

    void GameServer::SendStateSnapshots() {
        StatePacketHeader header;
        header.tick = m_currentTick;
        header.timestamp = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());
        header.entityCount = static_cast<uint16_t>(m_entities.size());
        header.scrollOffset = m_scrollOffset; // Send scroll position to clients

        std::vector<uint8_t> packet(sizeof(StatePacketHeader) + sizeof(EntityState) * m_entities.size());
        std::memcpy(packet.data(), &header, sizeof(StatePacketHeader));

        size_t offset = sizeof(StatePacketHeader);
        for (const auto& entity : m_entities) {
            EntityState state;
            state.entityId = entity.id;
            state.entityType = static_cast<uint8_t>(entity.type);
            state.x = entity.x;
            state.y = entity.y;
            state.vx = entity.vx;
            state.vy = entity.vy;
            state.health = entity.health;
            state.flags = entity.flags;
            state.ownerHash = entity.ownerHash; // For client-side prediction

            std::memcpy(packet.data() + offset, &state, sizeof(EntityState));
            offset += sizeof(EntityState);
        }

        static int sendCount = 0;
        if (sendCount++ % 60 == 0) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            std::cout << "[SERVER SEND] t=" << ms << " tick=" << m_currentTick
                      << " Broadcasting state to " << m_connectedPlayers.size() << " clients" << std::endl;
        }

        Broadcast(packet);
    }

    void GameServer::SendTo(const std::vector<uint8_t>& data, const Endpoint& to) {
        try {
            m_socket.send_to(asio::buffer(data), to.raw());
            m_packetsSent++;
        } catch (const std::exception& e) {
            std::cerr << "Error sending to " << to.address() << ":" << to.port() << " - " << e.what() << std::endl;
        }
    }

    void GameServer::Broadcast(const std::vector<uint8_t>& data) {
        for (const auto& [hash, player] : m_connectedPlayers) {
            SendTo(data, player.endpoint);
        }
    }

    void GameServer::UpdateGameLogic(float dt) {
        m_scrollOffset += SCROLL_SPEED * dt;

        UpdateMovement(dt);
        UpdateBullets(dt);
        UpdateEnemies(dt);
        CheckCollisions();
        CleanupDeadEntities();

        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - m_lastSpawnTime).count();
        if (elapsed >= m_enemySpawnInterval) {
            SpawnEnemy();
            m_lastSpawnTime = now;
        }
    }

    void GameServer::SpawnPlayer(uint64_t hash, float x, float y) {
        GameEntity player;
        player.id = GetNextEntityId();
        player.type = EntityType::PLAYER;
        player.x = x;
        player.y = y;
        player.vx = 0.0f;
        player.vy = 0.0f;
        player.health = 100;
        player.flags = 0;
        player.ownerHash = hash;

        m_entities.push_back(player);
        std::cout << "[Server] Spawned player entity " << player.id << " for playerHash=" << hash << " at (" << x << "," << y << ")" << std::endl;
    }

    void GameServer::SpawnEnemy() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> yDist(50.0f, 550.0f);

        GameEntity enemy;
        enemy.id = GetNextEntityId();
        enemy.type = EntityType::ENEMY;
        enemy.x = 1920.0f;
        enemy.y = yDist(gen);
        enemy.vx = -100.0f;
        enemy.vy = 0.0f;
        enemy.health = 50;
        enemy.flags = 0;
        enemy.ownerHash = 0;

        m_entities.push_back(enemy);
    }

    void GameServer::SpawnBullet(uint64_t ownerHash, float x, float y) {
        GameEntity bullet;
        bullet.id = GetNextEntityId();
        bullet.type = EntityType::BULLET;
        bullet.x = x;
        bullet.y = y;
        bullet.vx = 500.0f;
        bullet.vy = 0.0f;
        bullet.health = 1;
        bullet.flags = 0;
        bullet.ownerHash = ownerHash;

        m_entities.push_back(bullet);
    }

    void GameServer::UpdateMovement(float dt) {
        for (auto& entity : m_entities) {
            entity.x += entity.vx * dt;
            entity.y += entity.vy * dt;

            if (entity.x < -100.0f || entity.x > 2020.0f || entity.y < -100.0f || entity.y > 700.0f) {
                entity.health = 0;
            }
        }
    }

    void GameServer::UpdateBullets(float /*dt*/) {
    }

    void GameServer::UpdateEnemies(float /*dt*/) {
    }

    void GameServer::CheckCollisions() {
        auto bullets = GetEntitiesByType(EntityType::BULLET);
        auto enemies = GetEntitiesByType(EntityType::ENEMY);

        for (auto* bullet : bullets) {
            if (bullet->health == 0)
                continue;

            for (auto* enemy : enemies) {
                if (enemy->health == 0)
                    continue;

                float dx = bullet->x - enemy->x;
                float dy = bullet->y - enemy->y;
                float distSq = dx * dx + dy * dy;

                if (distSq < 50.0f * 50.0f) {
                    bullet->health = 0;
                    enemy->health = std::max(0, enemy->health - 10);
                }
            }
        }
    }

    void GameServer::CleanupDeadEntities() {
        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                           [](const GameEntity& e) { return e.health == 0; }),
            m_entities.end());
    }

    GameEntity* GameServer::FindEntityById(uint32_t id) {
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [id](const GameEntity& e) { return e.id == id; });
        return it != m_entities.end() ? &(*it) : nullptr;
    }

    std::vector<GameEntity*> GameServer::GetEntitiesByType(EntityType type) {
        std::vector<GameEntity*> result;
        for (auto& entity : m_entities) {
            if (entity.type == type) {
                result.push_back(&entity);
            }
        }
        return result;
    }

}
