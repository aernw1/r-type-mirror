/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameServer implementation
*/

#include "GameServer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <random>
#include <cmath>

using json = nlohmann::json;

namespace network {

<<<<<<< HEAD
    namespace {
        void BasicMovementPattern(GameEntity& enemy, float /*dt*/) {
            enemy.vx = -220.0f;
            enemy.vy = 0.0f;
        }

        void FastMovementPattern(GameEntity& enemy, float /*dt*/) {
            enemy.vx = -200.0f;
            enemy.vy = std::sin(enemy.x * 0.01f) * 50.0f;
        }

        void TankMovementPattern(GameEntity& enemy, float /*dt*/) {
            enemy.vx = -220.0f;
            enemy.vy = 0.0f;
        }

        void BossMovementPattern(GameEntity& enemy, float /*dt*/) {
            enemy.vx = -75.0f;
            enemy.vy = 0.0f;
        }

        void FormationMovementPattern(GameEntity& enemy, float /*dt*/) {
            enemy.vx = -100.0f;
            enemy.vy = 0.0f;
        }
    }

    const std::array<EnemyStats, 5> GameServer::s_enemyStats = {{
        {
            220.0f,  // speed
            100,     // health
            8,       // damage
            1.0f,    // fireRate
            -50.0f,  // bulletXOffset
            25.0f,   // bulletYOffset
            25,      // collisionDamageMultiplier (2.5x)
            BasicMovementPattern
        },
        {
            200.0f,
            50,
            3,
            0.5f,
            -50.0f,
            20.0f,
            20,
            FastMovementPattern
        },
        {
            220.0f,
            200,
            18,
            1.8f,
            -30.0f,
            -20.0f,
            30,
            TankMovementPattern
        },
        {
            75.0f,
            255,
            50,
            0.5f,
            -30.0f,
            45.0f,
            50,
            BossMovementPattern
        },
        {
            100.0f,
            100,
            10,
            1.5f,
            -30.0f,
            45.0f,
            25,
            FormationMovementPattern
        }
    }};

    GameServer::GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers,
                           const std::string& levelPath)
        : m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
          m_expectedPlayers(expectedPlayers),
          m_lastSpawnTime(std::chrono::steady_clock::now()),
          m_levelPath(levelPath) {

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

        const float defaultSpawnY[] = {200.0f, 360.0f, 520.0f, 680.0f};
        size_t playerIndex = 0;
        for (const auto& [hash, player] : m_connectedPlayers) {
            float spawnX = 100.0f;
            float spawnY = defaultSpawnY[playerIndex % 4];

            SpawnPlayer(hash, spawnX, spawnY);
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
            SpawnBullet(input->playerHash, playerEntity->x + 70.0f, playerEntity->y + 50.0f);
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
        header.scrollOffset = m_scrollOffset;

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
            state.ownerHash = entity.ownerHash;

            std::memcpy(packet.data() + offset, &state, sizeof(EntityState));
            offset += sizeof(EntityState);
        }

        static int sendCount = 0;
        if (sendCount++ % 60 == 0) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();
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

        EnemyType enemyType = GetRandomEnemyType();
        const EnemyStats& stats = GetEnemyStats(enemyType);
        float spawnX = 1920.0f;
        float spawnY = yDist(gen);

        GameEntity enemy;
        enemy.id = GetNextEntityId();
        enemy.type = EntityType::ENEMY;
        enemy.x = spawnX;
        enemy.y = spawnY;
        enemy.vx = -stats.speed;
        enemy.vy = 0.0f;
        enemy.health = stats.health;
        enemy.flags = static_cast<uint8_t>(enemyType);
        enemy.ownerHash = 0;

        m_entities.push_back(enemy);
        m_enemyShootCooldowns[enemy.id] = 0.0f;

        std::cout << "[Server] Spawned enemy type " << static_cast<int>(enemyType)
                  << " (id=" << enemy.id << ") at (" << spawnX << "," << spawnY << ")" << std::endl;
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

    void GameServer::SpawnEnemyBullet(uint32_t enemyId, float x, float y) {
        GameEntity* enemy = FindEntityById(enemyId);
        if (!enemy || enemy->health == 0)
            return;

        EnemyType enemyType = static_cast<EnemyType>(enemy->flags);

        GameEntity bullet;
        bullet.id = GetNextEntityId();
        bullet.type = EntityType::BULLET;
        bullet.x = x;
        bullet.y = y;
        bullet.vx = -400.0f;
        bullet.vy = 0.0f;
        bullet.health = 1;
        bullet.flags = 10 + static_cast<uint8_t>(enemyType);
        bullet.ownerHash = 0;

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
        const float MAX_X = 2000.0f;
        const float MIN_X = -200.0f;
        const float MAX_Y = 1000.0f;
        const float MIN_Y = -200.0f;

        auto bullets = GetEntitiesByType(EntityType::BULLET);
        for (auto* bullet : bullets) {
            if (bullet->x > MAX_X || bullet->x < MIN_X || bullet->y > MAX_Y || bullet->y < MIN_Y) {
                bullet->health = 0;
            }
        }
    }

    void GameServer::UpdateEnemies(float dt) {
        auto enemies = GetEntitiesByType(EntityType::ENEMY);

        for (auto* enemy : enemies) {
            if (enemy->health == 0)
                continue;

            EnemyType type = static_cast<EnemyType>(enemy->flags);
            const EnemyStats& stats = GetEnemyStats(type);

            stats.movementPattern(*enemy, dt);

            if (m_enemyShootCooldowns.find(enemy->id) == m_enemyShootCooldowns.end()) {
                m_enemyShootCooldowns[enemy->id] = 0.0f;
            }

            m_enemyShootCooldowns[enemy->id] -= dt;

            if (m_enemyShootCooldowns[enemy->id] <= 0.0f && HasPlayerInSight(*enemy)) {
                SpawnEnemyBullet(enemy->id, enemy->x + stats.bulletXOffset, enemy->y + stats.bulletYOffset);
                m_enemyShootCooldowns[enemy->id] = stats.fireRate;
            }

            if (enemy->x < -100.0f) {
                enemy->health = 0;
            }
        }

        for (auto it = m_enemyShootCooldowns.begin(); it != m_enemyShootCooldowns.end();) {
            if (FindEntityById(it->first) == nullptr || FindEntityById(it->first)->health == 0) {
                it = m_enemyShootCooldowns.erase(it);
            } else {
                ++it;
            }
        }
    }

    void GameServer::CheckCollisions() {
        auto bullets = GetEntitiesByType(EntityType::BULLET);
        auto enemies = GetEntitiesByType(EntityType::ENEMY);
        auto players = GetEntitiesByType(EntityType::PLAYER);

        const float COLLISION_RADIUS = 25.0f;
        const float COLLISION_RADIUS_SQ = COLLISION_RADIUS * COLLISION_RADIUS;

        for (auto* bullet : bullets) {
            if (bullet->health == 0)
                continue;
            if (bullet->flags != 0)
                continue;

            for (auto* enemy : enemies) {
                if (enemy->health == 0)
                    continue;

                float dx = bullet->x - enemy->x;
                float dy = bullet->y - enemy->y;
                float distSq = dx * dx + dy * dy;

                if (distSq < COLLISION_RADIUS_SQ) {
                    const uint8_t BULLET_DAMAGE = 10;

                    bullet->health = 0;
                    if (enemy->health > BULLET_DAMAGE) {
                        enemy->health -= BULLET_DAMAGE;
                    } else {
                        enemy->health = 0;
                    }
                }
            }
        }

        for (auto* bullet : bullets) {
            if (bullet->health == 0)
                continue;
            if (bullet->flags < 10)
                continue;

            for (auto* player : players) {
                if (player->health == 0)
                    continue;

                float dx = bullet->x - player->x;
                float dy = bullet->y - player->y;
                float distSq = dx * dx + dy * dy;

                if (distSq < COLLISION_RADIUS_SQ) {
                    uint8_t enemyBulletType = bullet->flags - 10;
                    EnemyType enemyType = static_cast<EnemyType>(enemyBulletType);
                    const EnemyStats& stats = GetEnemyStats(enemyType);

                    bullet->health = 0;
                    if (player->health > stats.damage) {
                        player->health -= stats.damage;
                    } else {
                        player->health = 0;
                    }
                }
            }
        }

        for (auto* enemy : enemies) {
            if (enemy->health == 0)
                continue;

            for (auto* player : players) {
                if (player->health == 0)
                    continue;

                float dx = enemy->x - player->x;
                float dy = enemy->y - player->y;
                float distSq = dx * dx + dy * dy;

                if (distSq < COLLISION_RADIUS_SQ * 1.5f) {
                    EnemyType enemyType = static_cast<EnemyType>(enemy->flags);
                    const EnemyStats& stats = GetEnemyStats(enemyType);
                    uint8_t collisionDamage = (stats.damage * stats.collisionDamageMultiplier) / 10;

                    enemy->health = 0;
                    if (player->health > collisionDamage) {
                        player->health -= collisionDamage;
                    } else {
                        player->health = 0;
                    }
                }
            }
        }
    }

    void GameServer::CleanupDeadEntities() {
        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                           [this](const GameEntity& e) {
                               if (e.health == 0 && e.type == EntityType::ENEMY) {
                                   m_enemyShootCooldowns.erase(e.id);
                               }
                               return e.health == 0;
                           }),
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

    EnemyType GameServer::GetRandomEnemyType() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 2);
        return static_cast<EnemyType>(dist(gen));
    }

    const EnemyStats& GameServer::GetEnemyStats(EnemyType type) const {
        size_t index = static_cast<size_t>(type);
        if (index >= s_enemyStats.size()) {
            index = 0;
        }
        return s_enemyStats[index];
    }

    bool GameServer::HasPlayerInSight(const GameEntity& enemy) {
        auto players = GetEntitiesByType(EntityType::PLAYER);
        const float SIGHT_RANGE = 800.0f;
        const float Y_TOLERANCE = 100.0f;

        for (auto* player : players) {
            if (player->health == 0)
                continue;

            float dx = player->x - enemy.x;
            if (dx > 0.0f || dx < -SIGHT_RANGE)
                continue;

            float dy = std::abs(player->y - enemy.y);
            if (dy <= Y_TOLERANCE) {
                return true;
            }
        }

        return false;
    }

}
