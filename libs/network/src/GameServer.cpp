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

    const std::array<EnemyStats, 5> GameServer::s_enemyStats = {{{220.0f, 100, 8, 1.0f, -50.0f, 25.0f, 25, BasicMovementPattern}, {200.0f, 50, 3, 0.5f, -50.0f, 20.0f, 20, FastMovementPattern}, {220.0f, 200, 18, 1.8f, -30.0f, -20.0f, 30, TankMovementPattern}, {75.0f, 255, 50, 0.5f, -30.0f, 45.0f, 50, BossMovementPattern}, {100.0f, 100, 10, 1.5f, -30.0f, 45.0f, 25, FormationMovementPattern}}};

    GameServer::GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers, const std::string& levelPath) : m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), m_expectedPlayers(expectedPlayers), m_lastSpawnTime(std::chrono::steady_clock::now()), m_levelPath(levelPath) {

        asio::socket_base::send_buffer_size sendOption(1024 * 1024);
        m_socket.set_option(sendOption);

        m_scrollingSystem = std::make_unique<RType::ECS::ScrollingSystem>();
        m_movementSystem = std::make_unique<RType::ECS::MovementSystem>();
        m_collisionDetectionSystem = std::make_unique<RType::ECS::CollisionDetectionSystem>();
        m_bulletResponseSystem = std::make_unique<RType::ECS::BulletCollisionResponseSystem>();
        m_playerResponseSystem = std::make_unique<RType::ECS::PlayerCollisionResponseSystem>();
        m_obstacleResponseSystem = std::make_unique<RType::ECS::ObstacleCollisionResponseSystem>();
        m_healthSystem = std::make_unique<RType::ECS::HealthSystem>();

        std::cout << "GameServer started on UDP port " << port << std::endl;
        std::cout << "ECS collision systems initialized" << std::endl;
        std::cout << "Waiting for " << expectedPlayers.size() << " players..." << std::endl;
    }

    GameServer::~GameServer() {
        Stop();
    }

    void GameServer::Run() {
        m_running = true;

        WaitForAllPlayers();

        try {
            std::cout << "Loading level from: " << m_levelPath << std::endl;
            auto levelData = RType::ECS::LevelLoader::LoadFromFile(m_levelPath);
            std::cout << "Level JSON loaded: " << levelData.obstacles.size() << " obstacle definitions found" << std::endl;

            auto createdEntities = RType::ECS::LevelLoader::CreateServerEntities(m_registry, levelData);
            std::cout << "Level loaded: " << createdEntities.obstacleColliders.size() << " obstacle colliders, " << createdEntities.enemies.size() << " enemy entities created" << std::endl;

            size_t obstaclesWithColliders = 0;
            for (auto obsEntity : createdEntities.obstacleColliders) {
                if (m_registry.HasComponent<RType::ECS::Obstacle>(obsEntity) && m_registry.HasComponent<RType::ECS::BoxCollider>(obsEntity) && m_registry.HasComponent<RType::ECS::CollisionLayer>(obsEntity)) {
                    obstaclesWithColliders++;
                }
            }
            std::cout << "Verified: " << obstaclesWithColliders << " obstacles have collision components" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load level: " << e.what() << std::endl;
            std::cerr << "Continuing without obstacles..." << std::endl;
        }

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

            if (AllPlayersDisconnected()) {
                std::cout << "All players disconnected. Stopping game server..." << std::endl;
                Stop();
                break;
            }

            UpdateGameLogic(TICK_RATE);
            m_currentTick++;

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

    bool GameServer::AllPlayersDisconnected() const {
        if (m_connectedPlayers.empty()) {
            return true;
        }

        auto now = std::chrono::steady_clock::now();
        for (const auto& [hash, player] : m_connectedPlayers) {
            auto timeSinceLastPing = std::chrono::duration_cast<std::chrono::seconds>(
                now - player.lastPingTime);
            if (timeSinceLastPing < DISCONNECT_TIMEOUT) {
                return false;
            }
        }
        return true;
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

        auto it = std::find_if(m_expectedPlayers.begin(), m_expectedPlayers.end(), [hello](const PlayerInfo& p) { return p.hash == hello->playerHash; });

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

            std::cout << "Player " << it->name << " connected (" << m_connectedPlayers.size() << "/" << m_expectedPlayers.size() << ")" << std::endl;
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
            std::cout << "[SERVER RECV INPUT] t=" << ms << " from=" << it->second.info.name << " inputs=" << (int)input->inputs << " seq=" << input->sequence << std::endl;
        }

        it->second.lastInputSequence = input->sequence;
        it->second.lastPingTime = now;

        using namespace RType::ECS;
        RType::ECS::Entity playerEntity = NULL_ENTITY;
        auto players = m_registry.GetEntitiesWithComponent<Player>();
        for (auto entity : players) {
            if (!m_registry.HasComponent<Player>(entity))
                continue;
            const auto& player = m_registry.GetComponent<Player>(entity);
            if (player.playerHash == input->playerHash) {
                playerEntity = entity;
                break;
            }
        }

        if (playerEntity == NULL_ENTITY || !m_registry.HasComponent<Velocity>(playerEntity))
            return;

        const float SPEED = 300.0f;

        auto& vel = m_registry.GetComponent<Velocity>(playerEntity);
        vel.dx = 0.0f;
        vel.dy = 0.0f;

        if (input->inputs & InputFlags::UP)
            vel.dy = -SPEED;
        if (input->inputs & InputFlags::DOWN)
            vel.dy = SPEED;
        if (input->inputs & InputFlags::LEFT)
            vel.dx = -SPEED;
        if (input->inputs & InputFlags::RIGHT)
            vel.dx = SPEED;

        if (input->inputs & InputFlags::SHOOT) {
            if (m_registry.HasComponent<Position>(playerEntity)) {
                const auto& pos = m_registry.GetComponent<Position>(playerEntity);
                float offsetX = 50.0f;
                float offsetY = 0.0f;
                if (m_registry.HasComponent<Shooter>(playerEntity)) {
                    const auto& shooter = m_registry.GetComponent<Shooter>(playerEntity);
                    offsetX = shooter.offsetX;
                    offsetY = shooter.offsetY;
                }
                SpawnBullet(input->playerHash, pos.x + offsetX, pos.y + offsetY);
            }
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
        header.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
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
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            std::cout << "[SERVER SEND] t=" << ms << " tick=" << m_currentTick << " Broadcasting state to " << m_connectedPlayers.size() << " clients" << std::endl;
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

        m_scrollingSystem->Update(m_registry, dt);
        m_movementSystem->Update(m_registry, dt);

        UpdateBullets(dt);
        UpdateEnemies(dt);

        m_collisionDetectionSystem->Update(m_registry, dt);
        m_bulletResponseSystem->Update(m_registry, dt);
        m_playerResponseSystem->Update(m_registry, dt);
        m_obstacleResponseSystem->Update(m_registry, dt);
        m_healthSystem->Update(m_registry, dt);

        UpdateLegacyEntitiesFromRegistry();
        CleanupDeadEntities();

        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - m_lastSpawnTime).count();
        if (elapsed >= m_enemySpawnInterval) {
            SpawnEnemy();
            m_lastSpawnTime = now;
        }
    }

    void GameServer::SpawnPlayer(uint64_t hash, float x, float y) {
        uint8_t playerNumber = static_cast<uint8_t>(m_connectedPlayers.size());
        RType::ECS::Entity playerEntity = RType::ECS::PlayerFactory::CreatePlayer(m_registry, playerNumber, hash, x, y, nullptr); // No renderer on server
        std::cout << "[Server] Spawned ECS player entity for playerHash=" << hash << " at (" << x << "," << y << ")" << std::endl;
    }

    void GameServer::SpawnEnemy() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> yDist(50.0f, 550.0f);

        EnemyType enemyType = GetRandomEnemyType();
        const EnemyStats& stats = GetEnemyStats(enemyType);
        float spawnX = 1920.0f;
        float spawnY = yDist(gen);

        RType::ECS::EnemyType ecsEnemyType = static_cast<RType::ECS::EnemyType>(static_cast<uint8_t>(enemyType));

        RType::ECS::Entity enemyEntity = RType::ECS::EnemyFactory::CreateEnemy(m_registry, ecsEnemyType, spawnX, spawnY, nullptr);

        uint32_t enemyId = static_cast<uint32_t>(enemyEntity);
        m_enemyShootCooldowns[enemyId] = 0.0f;

        std::cout << "[Server] Spawned ECS enemy type " << static_cast<int>(enemyType) << " (entity=" << enemyId << ") at (" << spawnX << "," << spawnY << ")" << std::endl;
    }

    void GameServer::SpawnBullet(uint64_t ownerHash, float x, float y) {
        using namespace RType::ECS;
        Entity bulletEntity = m_registry.CreateEntity();
        m_registry.AddComponent<Position>(bulletEntity, Position(x, y));
        m_registry.AddComponent<Velocity>(bulletEntity, Velocity(500.0f, 0.0f));
        m_registry.AddComponent<Bullet>(bulletEntity, Bullet(NULL_ENTITY));
        m_registry.AddComponent<Damage>(bulletEntity, Damage(25));
        m_registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));
        m_registry.AddComponent<CircleCollider>(bulletEntity, CircleCollider(5.0f));
        m_registry.AddComponent<CollisionLayer>(bulletEntity, CollisionLayer(CollisionLayers::PLAYER_BULLET, CollisionLayers::ENEMY | CollisionLayers::OBSTACLE));
    }

    void GameServer::SpawnEnemyBullet(uint32_t enemyId, float x, float y) {
        using namespace RType::ECS;

        Entity enemyEntity = static_cast<Entity>(enemyId);

        if (!m_registry.IsEntityAlive(enemyEntity) || !m_registry.HasComponent<Enemy>(enemyEntity)) {
            return;
        }

        Entity bulletEntity = m_registry.CreateEntity();
        m_registry.AddComponent<Position>(bulletEntity, Position(x, y));
        m_registry.AddComponent<Velocity>(bulletEntity, Velocity(-400.0f, 0.0f));
        m_registry.AddComponent<Bullet>(bulletEntity, Bullet(NULL_ENTITY));
        m_registry.AddComponent<Damage>(bulletEntity, Damage(10));
        m_registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));
        m_registry.AddComponent<CircleCollider>(bulletEntity, CircleCollider(5.0f));
        m_registry.AddComponent<CollisionLayer>(bulletEntity, CollisionLayer(CollisionLayers::ENEMY_BULLET, CollisionLayers::PLAYER | CollisionLayers::OBSTACLE));
    }


    void GameServer::UpdateBullets(float /*dt*/) {
        using namespace RType::ECS;

        const float MAX_X = 2000.0f;
        const float MIN_X = -200.0f;
        const float MAX_Y = 1000.0f;
        const float MIN_Y = -200.0f;

        auto bullets = m_registry.GetEntitiesWithComponent<Bullet>();
        std::vector<Entity> bulletsToDestroy;

        for (auto bullet : bullets) {
            if (!m_registry.IsEntityAlive(bullet) || !m_registry.HasComponent<Position>(bullet)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(bullet);
            if (pos.x > MAX_X || pos.x < MIN_X || pos.y > MAX_Y || pos.y < MIN_Y) {
                bulletsToDestroy.push_back(bullet);
            }
        }

        for (auto bullet : bulletsToDestroy) {
            m_registry.DestroyEntity(bullet);
        }
    }

    void GameServer::UpdateEnemies(float dt) {
        using namespace RType::ECS;

        auto enemies = m_registry.GetEntitiesWithComponent<Enemy>();

        for (auto enemy : enemies) {
            if (!m_registry.IsEntityAlive(enemy)) {
                continue;
            }

            EnemySystem::ApplyMovementPattern(m_registry, enemy, dt);

            if (!m_registry.HasComponent<Enemy>(enemy) || !m_registry.HasComponent<Position>(enemy)) {
                continue;
            }

            const auto& enemyComp = m_registry.GetComponent<Enemy>(enemy);
            const auto& pos = m_registry.GetComponent<Position>(enemy);

            uint32_t enemyId = static_cast<uint32_t>(enemy);

            if (m_enemyShootCooldowns.find(enemyId) == m_enemyShootCooldowns.end()) {
                m_enemyShootCooldowns[enemyId] = 0.0f;
            }

            m_enemyShootCooldowns[enemyId] -= dt;

            if (m_enemyShootCooldowns[enemyId] <= 0.0f) {
                EnemyType type = static_cast<EnemyType>(static_cast<uint8_t>(enemyComp.type));
                const EnemyStats& stats = GetEnemyStats(type);
                SpawnEnemyBullet(enemyId, pos.x + stats.bulletXOffset, pos.y + stats.bulletYOffset);
                m_enemyShootCooldowns[enemyId] = stats.fireRate;
            }
        }

        EnemySystem::DestroyEnemiesOffScreen(m_registry, 1280.0f);

        for (auto it = m_enemyShootCooldowns.begin(); it != m_enemyShootCooldowns.end();) {
            Entity enemyEntity = static_cast<Entity>(it->first);
            if (!m_registry.IsEntityAlive(enemyEntity) || !m_registry.HasComponent<Enemy>(enemyEntity)) {
                it = m_enemyShootCooldowns.erase(it);
            } else {
                ++it;
            }
        }
    }

    void GameServer::CleanupDeadEntities() {
        m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [this](const GameEntity& e) { if (e.health == 0 && e.type == EntityType::ENEMY) { m_enemyShootCooldowns.erase(e.id); } return e.health == 0; }), m_entities.end());
    }

    void GameServer::UpdateLegacyEntitiesFromRegistry() {
        using namespace RType::ECS;

        m_entities.clear();

        auto players = m_registry.GetEntitiesWithComponent<Player>();
        for (auto playerEntity : players) {
            if (!m_registry.IsEntityAlive(playerEntity) ||
                !m_registry.HasComponent<Position>(playerEntity) ||
                !m_registry.HasComponent<Velocity>(playerEntity) ||
                !m_registry.HasComponent<Health>(playerEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(playerEntity);
            const auto& vel = m_registry.GetComponent<Velocity>(playerEntity);
            const auto& health = m_registry.GetComponent<Health>(playerEntity);
            const auto& player = m_registry.GetComponent<Player>(playerEntity);

            GameEntity entity;
            entity.id = static_cast<uint32_t>(playerEntity);
            entity.type = EntityType::PLAYER;
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = static_cast<uint8_t>(std::min(255, std::max(0, health.current)));
            entity.flags = player.playerNumber;
            entity.ownerHash = player.playerHash;
            m_entities.push_back(entity);
        }

        auto enemies = m_registry.GetEntitiesWithComponent<Enemy>();
        for (auto enemyEntity : enemies) {
            if (!m_registry.IsEntityAlive(enemyEntity) ||
                !m_registry.HasComponent<Position>(enemyEntity) ||
                !m_registry.HasComponent<Velocity>(enemyEntity) ||
                !m_registry.HasComponent<Health>(enemyEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(enemyEntity);
            const auto& vel = m_registry.GetComponent<Velocity>(enemyEntity);
            const auto& health = m_registry.GetComponent<Health>(enemyEntity);
            const auto& enemy = m_registry.GetComponent<Enemy>(enemyEntity);

            GameEntity entity;
            entity.id = static_cast<uint32_t>(enemyEntity);
            entity.type = EntityType::ENEMY;
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = static_cast<uint8_t>(std::min(255, std::max(0, health.current)));
            entity.flags = static_cast<uint8_t>(enemy.type);
            entity.ownerHash = 0;
            m_entities.push_back(entity);
        }

        auto bullets = m_registry.GetEntitiesWithComponent<Bullet>();
        for (auto bulletEntity : bullets) {
            if (!m_registry.IsEntityAlive(bulletEntity) ||
                !m_registry.HasComponent<Position>(bulletEntity) ||
                !m_registry.HasComponent<Velocity>(bulletEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(bulletEntity);
            const auto& vel = m_registry.GetComponent<Velocity>(bulletEntity);
            const auto& bullet = m_registry.GetComponent<Bullet>(bulletEntity);

            uint8_t flags = 0;
            if (m_registry.HasComponent<CollisionLayer>(bulletEntity)) {
                const auto& collLayer = m_registry.GetComponent<CollisionLayer>(bulletEntity);
                if (collLayer.layer == CollisionLayers::ENEMY_BULLET) {
                    flags = 10;
                }
            }

            GameEntity entity;
            entity.id = static_cast<uint32_t>(bulletEntity);
            entity.type = EntityType::BULLET;
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = 1;
            entity.flags = flags;
            entity.ownerHash = 0;
            m_entities.push_back(entity);
        }

        auto obstacles = m_registry.GetEntitiesWithComponent<Obstacle>();
        const size_t maxObstaclesPerSnapshot = 64;
        size_t obstacleCount = 0;
        for (auto obstacleEntity : obstacles) {
            if (!m_registry.IsEntityAlive(obstacleEntity) ||
                !m_registry.HasComponent<Position>(obstacleEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(obstacleEntity);

            GameEntity entity;
            entity.id = static_cast<uint32_t>(obstacleEntity);
            entity.type = EntityType::OBSTACLE;
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = 0.0f;
            entity.vy = 0.0f;
            entity.health = 255;
            entity.flags = 0;
            uint64_t obstacleIndex = 0;
            if (m_registry.HasComponent<RType::ECS::ObstacleMetadata>(obstacleEntity)) {
                obstacleIndex = m_registry.GetComponent<RType::ECS::ObstacleMetadata>(obstacleEntity).uniqueId;
            }
            entity.ownerHash = obstacleIndex;
            m_entities.push_back(entity);
            obstacleCount++;
            if (obstacleCount >= maxObstaclesPerSnapshot) {
                break;
            }
        }
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

}
