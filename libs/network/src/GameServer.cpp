/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameServer implementation
*/

#include "GameServer.hpp"
#include "ECS/BossSystem.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <random>
#include <cmath>

using json = nlohmann::json;

namespace network {

    uint32_t GameServer::GetOrAssignNetworkId(RType::ECS::Entity entity)
    {
        // CRITICAL: ECS entity IDs are recycled, sometimes within the same tick.
        // Therefore, network IDs must be stored on the entity itself (as a component),
        // not in a map keyed only by the raw ECS entity ID.
        if (m_registry.HasComponent<RType::ECS::NetworkId>(entity)) {
            return m_registry.GetComponent<RType::ECS::NetworkId>(entity).id;
        }
        const uint32_t id = m_nextNetworkId++;
        m_registry.AddComponent<RType::ECS::NetworkId>(entity, RType::ECS::NetworkId{id});
        return id;
    }

    void GameServer::RegisterNetworkType(uint32_t netId, EntityType type)
    {
        auto it = m_networkIdTypes.find(netId);
        if (it == m_networkIdTypes.end()) {
            m_networkIdTypes.emplace(netId, type);
            return;
        }
        if (it->second != type) {
            std::cerr << "[NETID COLLISION] NetworkId " << netId
                      << " was " << static_cast<int>(it->second)
                      << " now " << static_cast<int>(type)
                      << " -- this indicates ID reuse/type confusion" << std::endl;
            it->second = type;
        }
    }

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

    GameServer::GameServer(Network::INetworkModule* network, uint16_t port,
        const std::vector<PlayerInfo>& expectedPlayers, const std::string& levelPath)
        : m_network(network), m_expectedPlayers(expectedPlayers),
          m_lastSpawnTime(std::chrono::steady_clock::now()), m_levelPath(levelPath) {

        m_udpSocket = m_network->CreateUdpSocket();
        m_network->BindUdp(m_udpSocket, port);

        m_scrollingSystem = std::make_unique<RType::ECS::ScrollingSystem>();
        m_bossSystem = std::make_unique<RType::ECS::BossSystem>();
        m_bossAttackSystem = std::make_unique<RType::ECS::BossAttackSystem>();
        m_blackOrbSystem = std::make_unique<RType::ECS::BlackOrbSystem>();
        m_thirdBulletSystem = std::make_unique<RType::ECS::ThirdBulletSystem>();
        m_movementSystem = std::make_unique<RType::ECS::MovementSystem>();
        m_collisionDetectionSystem = std::make_unique<RType::ECS::CollisionDetectionSystem>();
        m_bulletResponseSystem = std::make_unique<RType::ECS::BulletCollisionResponseSystem>();
        m_playerResponseSystem = std::make_unique<RType::ECS::PlayerCollisionResponseSystem>();
        m_obstacleResponseSystem = std::make_unique<RType::ECS::ObstacleCollisionResponseSystem>();
        m_healthSystem = std::make_unique<RType::ECS::HealthSystem>();
        m_scoreSystem = std::make_unique<RType::ECS::ScoreSystem>();
        m_powerUpSpawnSystem = std::make_unique<RType::ECS::PowerUpSpawnSystem>(
            nullptr, // No renderer on server
            1280.0f, // screenWidth
            720.0f   // screenHeight
        );
        m_powerUpSpawnSystem->SetSpawnInterval(5.0f); // Spawn every 5 seconds for testing
        m_powerUpCollisionSystem = std::make_unique<RType::ECS::PowerUpCollisionSystem>(nullptr);

        // Shooting system for spread shot and laser beam
        // Note: We use INVALID_SPRITE_ID since we don't need rendering on server
        m_shootingSystem = std::make_unique<RType::ECS::ShootingSystem>(0);

        // Force pod and shield systems
        m_forcePodSystem = std::make_unique<RType::ECS::ForcePodSystem>();
        m_shieldSystem = std::make_unique<RType::ECS::ShieldSystem>();

        std::cout << "GameServer started on UDP port " << port << std::endl;
        std::cout << "ECS collision systems initialized" << std::endl;
        std::cout << "Waiting for " << expectedPlayers.size() << " players..." << std::endl;
    }

    GameServer::~GameServer() {
        Stop();
        if (m_network && m_udpSocket != Network::INVALID_SOCKET_ID) {
            m_network->CloseSocket(m_udpSocket);
            m_udpSocket = Network::INVALID_SOCKET_ID;
        }
    }

    void GameServer::Run() {
        m_running = true;

        WaitForAllPlayers();

        try {
            std::cout << "Loading level from: " << m_levelPath << std::endl;
            auto levelData = RType::ECS::LevelLoader::LoadFromFile(m_levelPath);
            std::cout << "Level JSON loaded: " << levelData.obstacles.size() << " obstacle definitions found" << std::endl;
            std::cout << "Boss in level data: " << (levelData.boss.has_value() ? "YES" : "NO") << std::endl;
            if (levelData.boss.has_value()) {
                std::cout << "Boss position: (" << levelData.boss->x << ", " << levelData.boss->y << ")" << std::endl;
            }

            auto createdEntities = RType::ECS::LevelLoader::CreateServerEntities(m_registry, levelData);
            std::cout << "Level loaded: " << createdEntities.obstacleColliders.size() << " obstacle colliders, "
                      << createdEntities.enemies.size() << " enemy entities, boss: "
                      << (createdEntities.boss != RType::ECS::NULL_ENTITY ? "CREATED" : "NOT CREATED") << std::endl;

            // Initialize collider positions based on their visual entities
            for (auto collider : createdEntities.obstacleColliders) {
                if (!m_registry.IsEntityAlive(collider) ||
                    !m_registry.HasComponent<RType::ECS::ObstacleMetadata>(collider)) {
                    continue;
                }
                const auto& metadata = m_registry.GetComponent<RType::ECS::ObstacleMetadata>(collider);

                // Sync collider to visual entity position on initialization
                if (metadata.visualEntity != RType::ECS::NULL_ENTITY &&
                    m_registry.IsEntityAlive(metadata.visualEntity) &&
                    m_registry.HasComponent<RType::ECS::Position>(metadata.visualEntity) &&
                    m_registry.HasComponent<RType::ECS::Position>(collider)) {

                    const auto& visualPos = m_registry.GetComponent<RType::ECS::Position>(metadata.visualEntity);
                    auto& colliderPos = m_registry.GetComponent<RType::ECS::Position>(collider);

                    // Set absolute position = visual position + offset
                    colliderPos.x = visualPos.x + metadata.offsetX;
                    colliderPos.y = visualPos.y + metadata.offsetY;

                    std::cout << "[SERVER INIT] Collider synced: visual=(" << visualPos.x << "," << visualPos.y
                              << ") offset=(" << metadata.offsetX << "," << metadata.offsetY
                              << ") -> collider=(" << colliderPos.x << "," << colliderPos.y << ")" << std::endl;
                }
            }

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
            lastTick = now;

            ProcessIncomingPackets();

            // Check if we need to load next level when all players disconnect after boss defeat
            LoadNextLevelIfNeeded();

            if (AllPlayersDisconnected() && !m_levelComplete) {
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
        while (m_connectedPlayers.size() < m_expectedPlayers.size()) {
            auto packet = m_network->ReceiveUdp(m_udpSocket, 1024);
            if (packet && !packet->data.empty()) {
                HandleHello(packet->data, packet->from);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "All players connected!" << std::endl;
    }

    void GameServer::ProcessIncomingPackets() {
        int packetsRead = 0;
        while (packetsRead < 100) {
            auto packet = m_network->ReceiveUdp(m_udpSocket, 1024);
            if (!packet) {
                break;
            }
            if (!packet->data.empty()) {
                HandlePacket(packet->data, packet->from);
                m_packetsReceived++;
                packetsRead++;
            }
        }
    }

    void GameServer::HandlePacket(const std::vector<uint8_t>& data, const Network::Endpoint& from) {
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
        case GamePacket::DISCONNECT:
            HandleDisconnect(data, from);
            break;
        default:
            break;
        }
    }

    void GameServer::HandleHello(const std::vector<uint8_t>& data, const Network::Endpoint& from) {
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

    void GameServer::HandleInput(const std::vector<uint8_t>& data, const Network::Endpoint& /*from*/) {
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

        if (m_registry.HasComponent<ShootCommand>(playerEntity)) {
            auto& shootCmd = m_registry.GetComponent<ShootCommand>(playerEntity);
            shootCmd.wantsToShoot = (input->inputs & InputFlags::SHOOT) != 0;
        }
    }

    void GameServer::HandlePing(const std::vector<uint8_t>& data, const Network::Endpoint& from) {
        if (data.size() < sizeof(PingPacket))
            return;

        const PingPacket* ping = reinterpret_cast<const PingPacket*>(data.data());

        PongPacket pong;
        pong.timestamp = ping->timestamp;

        std::vector<uint8_t> response(sizeof(PongPacket));
        std::memcpy(response.data(), &pong, sizeof(PongPacket));

        SendTo(response, from);
    }

    void GameServer::HandleDisconnect(const std::vector<uint8_t>& data, const Network::Endpoint& from) {
        for (auto it = m_connectedPlayers.begin(); it != m_connectedPlayers.end(); ++it) {
            if (it->second.endpoint == from) {
                std::cout << "[GameServer] Player " << it->second.info.name
                          << " (hash: " << it->first << ") disconnected gracefully" << std::endl;
                m_connectedPlayers.erase(it);
                std::cout << "[GameServer] Remaining players: " << m_connectedPlayers.size() << std::endl;
                return;
            }
        }
    }

    void GameServer::SendStateSnapshots() {
        std::vector<InputAck> inputAcks;
        auto players = m_registry.GetEntitiesWithComponent<RType::ECS::Player>();
        for (const auto& [hash, connPlayer] : m_connectedPlayers) {
            InputAck ack;
            ack.playerHash = hash;
            ack.lastProcessedSeq = connPlayer.lastInputSequence;
            ack.serverPosX = 0.0f;
            ack.serverPosY = 0.0f;

            for (auto playerEntity : players) {
                if (!m_registry.IsEntityAlive(playerEntity) ||
                    !m_registry.HasComponent<RType::ECS::Player>(playerEntity))
                    continue;
                const auto& player = m_registry.GetComponent<RType::ECS::Player>(playerEntity);
                if (player.playerHash == hash && m_registry.HasComponent<RType::ECS::Position>(playerEntity)) {
                    const auto& pos = m_registry.GetComponent<RType::ECS::Position>(playerEntity);
                    ack.serverPosX = pos.x;
                    ack.serverPosY = pos.y;
                    break;
                }
            }
            inputAcks.push_back(ack);
        }

        StatePacketHeader header;
        header.tick = m_currentTick;
        header.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
        header.entityCount = static_cast<uint16_t>(m_entities.size());
        header.scrollOffset = m_scrollOffset;
        header.inputAckCount = static_cast<uint8_t>(inputAcks.size());

        size_t packetSize = sizeof(StatePacketHeader) +
                           sizeof(InputAck) * inputAcks.size() +
                           sizeof(EntityState) * m_entities.size();
        std::vector<uint8_t> packet(packetSize);
        std::memcpy(packet.data(), &header, sizeof(StatePacketHeader));

        size_t offset = sizeof(StatePacketHeader);
        for (const auto& ack : inputAcks) {
            std::memcpy(packet.data() + offset, &ack, sizeof(InputAck));
            offset += sizeof(InputAck);
        }

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
            state.score = entity.score;
            state.powerUpFlags = entity.powerUpFlags;
            state.speedMultiplier = entity.speedMultiplier;
            state.weaponType = entity.weaponType;
            state.fireRate = entity.fireRate;

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

    void GameServer::SendTo(const std::vector<uint8_t>& data, const Network::Endpoint& to) {
        if (!m_network || m_udpSocket == Network::INVALID_SOCKET_ID) {
            return;
        }
        try {
            m_network->SendUdp(m_udpSocket, data, to);
            m_packetsSent++;
        } catch (const std::exception& e) {
            std::cerr << "Error sending to " << to.address << ":" << to.port << " - " << e.what() << std::endl;
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
        m_bossSystem->Update(m_registry, dt);
        m_bossAttackSystem->Update(m_registry, dt);
        m_blackOrbSystem->Update(m_registry, dt);
        m_thirdBulletSystem->Update(m_registry, dt);
        m_movementSystem->Update(m_registry, dt);

        // Powerup systems (server-side only)
        m_powerUpSpawnSystem->Update(m_registry, dt);
        m_powerUpCollisionSystem->Update(m_registry, dt);

        // Shooting system for weapon powerups (spread shot, laser)
        m_shootingSystem->Update(m_registry, dt);

        // Force pod and shield systems
        m_forcePodSystem->Update(m_registry, dt);
        m_shieldSystem->Update(m_registry, dt);

        UpdateBullets(dt);
        UpdateEnemies(dt);

        m_collisionDetectionSystem->Update(m_registry, dt);
        m_bulletResponseSystem->Update(m_registry, dt);
        m_playerResponseSystem->Update(m_registry, dt);
        m_obstacleResponseSystem->Update(m_registry, dt);
        m_scoreSystem->Update(m_registry, dt);
        m_healthSystem->Update(m_registry, dt);

        // Check if boss is defeated for level progression
        CheckBossDefeated();

        UpdateLegacyEntitiesFromRegistry();
        CleanupDeadEntities();

        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - m_lastSpawnTime).count();

        // Don't spawn normal enemies when boss is active
        if (elapsed >= m_enemySpawnInterval && !IsBossActive()) {
            SpawnEnemy();
            m_lastSpawnTime = now;
        }
    }

    void GameServer::SpawnPlayer(uint64_t hash, float x, float y) {
        uint8_t playerNumber = static_cast<uint8_t>(m_connectedPlayers.size());
        RType::ECS::PlayerFactory::CreatePlayer(m_registry, playerNumber, hash, x, y, nullptr);
        std::cout << "[Server] Spawned ECS player entity for playerHash=" << hash << " at (" << x << "," << y << ")" << std::endl;
    }

    void GameServer::SpawnEnemy() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> yDist(50.0f, 550.0f);

        EnemyType enemyType = GetRandomEnemyType();
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

        // CRITICAL FIX: Clean up obstacle components from entity ID reuse
        if (m_registry.HasComponent<Obstacle>(bulletEntity)) {
            std::cerr << "[SERVER CLEANUP] Removing Obstacle from bullet entity " << bulletEntity << std::endl;
            m_registry.RemoveComponent<Obstacle>(bulletEntity);
        }
        if (m_registry.HasComponent<ObstacleMetadata>(bulletEntity)) {
            std::cerr << "[SERVER CLEANUP] Removing ObstacleMetadata from bullet entity " << bulletEntity << std::endl;
            m_registry.RemoveComponent<ObstacleMetadata>(bulletEntity);
        }

        m_registry.AddComponent<Position>(bulletEntity, Position(x, y));
        m_registry.AddComponent<Velocity>(bulletEntity, Velocity(500.0f, 0.0f));

        Entity ownerEntity = NULL_ENTITY;
        auto players = m_registry.GetEntitiesWithComponent<Player>();
        for (auto entity : players) {
            if (!m_registry.IsEntityAlive(entity) || !m_registry.HasComponent<Player>(entity)) {
                continue;
            }
            const auto& player = m_registry.GetComponent<Player>(entity);
            if (player.playerHash == ownerHash) {
                ownerEntity = entity;
                break;
            }
        }

        m_registry.AddComponent<Bullet>(bulletEntity, Bullet(ownerEntity));
        if (ownerEntity == NULL_ENTITY) {
            std::cout << "[Server] Warning: SpawnBullet could not resolve ownerHash=" << ownerHash
                      << " to an ECS player entity; bullet owner will be NULL_ENTITY" << std::endl;
        }
        m_registry.AddComponent<Damage>(bulletEntity, Damage(25));
        m_registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));
        m_registry.AddComponent<CircleCollider>(bulletEntity, CircleCollider(5.0f));
        m_registry.AddComponent<CollisionLayer>(bulletEntity, CollisionLayer(CollisionLayers::PLAYER_BULLET, CollisionLayers::ENEMY | CollisionLayers::OBSTACLE));
    }

    void GameServer::SpawnEnemyBullet(uint32_t enemyId, float x, float y, uint8_t enemyType) {
        using namespace RType::ECS;

        Entity enemyEntity = static_cast<Entity>(enemyId);

        if (!m_registry.IsEntityAlive(enemyEntity) || !m_registry.HasComponent<Enemy>(enemyEntity)) {
            return;
        }

        Entity bulletEntity = m_registry.CreateEntity();

        // CRITICAL FIX: Clean up obstacle components from entity ID reuse
        if (m_registry.HasComponent<Obstacle>(bulletEntity)) {
            std::cerr << "[SERVER CLEANUP] Removing Obstacle from bullet entity " << bulletEntity << std::endl;
            m_registry.RemoveComponent<Obstacle>(bulletEntity);
        }
        if (m_registry.HasComponent<ObstacleMetadata>(bulletEntity)) {
            std::cerr << "[SERVER CLEANUP] Removing ObstacleMetadata from bullet entity " << bulletEntity << std::endl;
            m_registry.RemoveComponent<ObstacleMetadata>(bulletEntity);
        }

        uint32_t bulletId = static_cast<uint32_t>(bulletEntity);
        m_registry.AddComponent<Position>(bulletEntity, Position(x, y));
        m_registry.AddComponent<Velocity>(bulletEntity, Velocity(-400.0f, 0.0f));
        m_registry.AddComponent<Bullet>(bulletEntity, Bullet(NULL_ENTITY));
        m_registry.AddComponent<Damage>(bulletEntity, Damage(10));
        m_registry.AddComponent<BoxCollider>(bulletEntity, BoxCollider(10.0f, 5.0f));
        m_registry.AddComponent<CircleCollider>(bulletEntity, CircleCollider(5.0f));
        m_registry.AddComponent<CollisionLayer>(bulletEntity, CollisionLayer(CollisionLayers::ENEMY_BULLET, CollisionLayers::PLAYER | CollisionLayers::OBSTACLE));

        m_enemyBulletTypes[bulletId] = enemyType;
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
            uint32_t bulletId = static_cast<uint32_t>(bullet);
            m_enemyBulletTypes.erase(bulletId);
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
                uint8_t enemyTypeValue = static_cast<uint8_t>(enemyComp.type);
                SpawnEnemyBullet(enemyId, pos.x + stats.bulletXOffset, pos.y + stats.bulletYOffset, enemyTypeValue);
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
        // NOTE: `GameEntity::id` is a stable *network* ID (not the ECS entity ID).
        // Never use it to index ECS-side maps like `m_enemyShootCooldowns`.
        m_entities.erase(
            std::remove_if(
                m_entities.begin(),
                m_entities.end(),
                [](const GameEntity& e) { return e.health == 0; }),
            m_entities.end());
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
            entity.id = GetOrAssignNetworkId(playerEntity);
            entity.type = EntityType::PLAYER;
            RegisterNetworkType(entity.id, entity.type);
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = static_cast<uint8_t>(std::min(255, std::max(0, health.current)));

            // Encode player number in flags (lower 4 bits)
            uint8_t flags = player.playerNumber & 0x0F;
            entity.flags = flags;
            entity.ownerHash = player.playerHash;

            if (m_registry.HasComponent<ScoreValue>(playerEntity)) {
                entity.score = m_registry.GetComponent<ScoreValue>(playerEntity).points;
            } else {
                entity.score = 0;
            }

            // Initialize power-up state defaults
            entity.powerUpFlags = 0;
            entity.speedMultiplier = 10; // 1.0 scaled by 10
            entity.weaponType = 0; // STANDARD
            entity.fireRate = 20; // 0.2 scaled by 10

            // Encode power-up state
            if (m_registry.HasComponent<ActivePowerUps>(playerEntity)) {
                const auto& powerUps = m_registry.GetComponent<ActivePowerUps>(playerEntity);

                if (powerUps.hasFireRateBoost) {
                    entity.powerUpFlags |= network::PowerUpFlags::POWERUP_FIRE_RATE_BOOST;
                }
                if (powerUps.hasSpreadShot) {
                    entity.powerUpFlags |= network::PowerUpFlags::POWERUP_SPREAD_SHOT;
                }
                if (powerUps.hasLaserBeam) {
                    entity.powerUpFlags |= network::PowerUpFlags::POWERUP_LASER_BEAM;
                }
                if (powerUps.hasShield) {
                    entity.powerUpFlags |= network::PowerUpFlags::POWERUP_SHIELD;
                }

                // Encode speed multiplier (scaled by 10, clamped to 0-255)
                float speedMult = powerUps.speedMultiplier;
                entity.speedMultiplier = static_cast<uint8_t>(std::min(255, std::max(0, static_cast<int>(speedMult * 10.0f))));
            }

            // Check if player has a force pod (separate entity with ForcePod component pointing to this player)
            auto forcePods = m_registry.GetEntitiesWithComponent<ForcePod>();
            for (auto podEntity : forcePods) {
                if (m_registry.IsEntityAlive(podEntity)) {
                    const auto& pod = m_registry.GetComponent<ForcePod>(podEntity);
                    if (pod.owner == playerEntity) {
                        entity.powerUpFlags |= network::PowerUpFlags::POWERUP_FORCE_POD;
                        break;
                    }
                }
            }

            // Encode weapon type and fire rate
            if (m_registry.HasComponent<WeaponSlot>(playerEntity)) {
                const auto& weaponSlot = m_registry.GetComponent<WeaponSlot>(playerEntity);
                entity.weaponType = static_cast<uint8_t>(weaponSlot.type);
                // Fire rate scaled by 10, clamped to 0-255
                entity.fireRate = static_cast<uint8_t>(std::min(255, std::max(0, static_cast<int>(weaponSlot.fireRate * 10.0f))));
            } else if (m_registry.HasComponent<Shooter>(playerEntity)) {
                // If no WeaponSlot, use Shooter fireRate (may be modified by fire rate boost)
                const auto& shooter = m_registry.GetComponent<Shooter>(playerEntity);
                entity.fireRate = static_cast<uint8_t>(std::min(255, std::max(0, static_cast<int>(shooter.fireRate * 10.0f))));
            }
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
            entity.id = GetOrAssignNetworkId(enemyEntity);
            entity.type = EntityType::ENEMY;
            RegisterNetworkType(entity.id, entity.type);
            entity.flags = static_cast<uint8_t>(enemy.type);
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = static_cast<uint8_t>(std::min(255, std::max(0, health.current)));
            entity.ownerHash = 0;
            entity.score = 0;
            m_entities.push_back(entity);
        }

        auto bosses = m_registry.GetEntitiesWithComponent<Boss>();
        for (auto bossEntity : bosses) {
            if (!m_registry.IsEntityAlive(bossEntity) ||
                !m_registry.HasComponent<Position>(bossEntity) ||
                !m_registry.HasComponent<Velocity>(bossEntity) ||
                !m_registry.HasComponent<Health>(bossEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(bossEntity);
            const auto& vel = m_registry.GetComponent<Velocity>(bossEntity);
            const auto& health = m_registry.GetComponent<Health>(bossEntity);

            uint8_t flags = 0;
            if (m_registry.HasComponent<RType::ECS::DamageFlash>(bossEntity)) {
                const auto& flash = m_registry.GetComponent<RType::ECS::DamageFlash>(bossEntity);
                if (flash.isActive) {
                    flags = 1;
                }
            }

            float healthPercent = (static_cast<float>(health.current) / static_cast<float>(health.max)) * 100.0f;
            uint8_t healthValue = static_cast<uint8_t>(std::min(100.0f, std::max(0.0f, healthPercent)));

            GameEntity entity;
            entity.id = GetOrAssignNetworkId(bossEntity);
            entity.type = EntityType::BOSS;
            RegisterNetworkType(entity.id, entity.type);
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = healthValue;
            entity.flags = flags;
            entity.ownerHash = 0;
            entity.score = 0;
            m_entities.push_back(entity);
        }

        auto bullets = m_registry.GetEntitiesWithComponent<Bullet>();
        // SAFETY NET: Clean up any contaminated bullets that accidentally carry obstacle data.
        for (auto bulletEntity : bullets) {
            if (m_registry.HasComponent<RType::ECS::Obstacle>(bulletEntity) ||
                m_registry.HasComponent<RType::ECS::ObstacleMetadata>(bulletEntity)) {
                std::cerr << "[SERVER CLEANUP] Bullet entity " << bulletEntity
                          << " had Obstacle components; removing to prevent obstacle desync." << std::endl;
                if (m_registry.HasComponent<RType::ECS::Obstacle>(bulletEntity)) {
                    m_registry.RemoveComponent<RType::ECS::Obstacle>(bulletEntity);
                }
                if (m_registry.HasComponent<RType::ECS::ObstacleMetadata>(bulletEntity)) {
                    m_registry.RemoveComponent<RType::ECS::ObstacleMetadata>(bulletEntity);
                }
            }
        }

        for (auto bulletEntity : bullets) {
            if (!m_registry.IsEntityAlive(bulletEntity) ||
                !m_registry.HasComponent<Position>(bulletEntity) ||
                !m_registry.HasComponent<Velocity>(bulletEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(bulletEntity);
            const auto& vel = m_registry.GetComponent<Velocity>(bulletEntity);
            const auto& bullet = m_registry.GetComponent<Bullet>(bulletEntity);
            (void)bullet;

            uint32_t bulletId = static_cast<uint32_t>(bulletEntity);
            uint8_t flags = 0;
            if (m_registry.HasComponent<RType::ECS::ThirdBullet>(bulletEntity)) {
                // Third Bullet gets flag 15
                flags = 15;
            } else if (m_registry.HasComponent<RType::ECS::BlackOrb>(bulletEntity)) {
                // Black Orb gets flag 14
                flags = 14;
            } else if (m_registry.HasComponent<RType::ECS::BossBullet>(bulletEntity)) {
                // Boss bullets get flag 13
                flags = 13;
            } else if (m_registry.HasComponent<CollisionLayer>(bulletEntity)) {
                const auto& collLayer = m_registry.GetComponent<CollisionLayer>(bulletEntity);
                if (collLayer.layer == CollisionLayers::ENEMY_BULLET) {
                    uint8_t enemyType = 0;
                    auto it = m_enemyBulletTypes.find(bulletId);
                    if (it != m_enemyBulletTypes.end()) {
                        enemyType = it->second;
                    }
                    flags = 10 + enemyType;
                }
            }

            GameEntity entity;
            entity.id = GetOrAssignNetworkId(bulletEntity);
            entity.type = EntityType::BULLET;
            RegisterNetworkType(entity.id, entity.type);
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = 1;
            entity.flags = flags;
            entity.ownerHash = 0;
            entity.score = 0;
            m_entities.push_back(entity);
        }

        auto obstacles = m_registry.GetEntitiesWithComponent<Obstacle>();
        const size_t maxObstaclesPerSnapshot = 256;  // Increased from 64 to support larger levels

        static bool loggedObstacleCount = false;
        if (!loggedObstacleCount) {
            std::cout << "[SERVER OBSTACLE SYNC] Total obstacles with Obstacle component: " << obstacles.size() << std::endl;
            loggedObstacleCount = true;
        }

        size_t obstacleCount = 0;
        static int serverObstacleLog = 0;
        for (auto obstacleEntity : obstacles) {
            if (!m_registry.IsEntityAlive(obstacleEntity) ||
                !m_registry.HasComponent<Position>(obstacleEntity)) {
                continue;
            }

            // Hard scrub: obstacles must be static colliders only.
            // If they carry Velocity/Bullet/Shooter/WeaponSlot, strip obstacle data and skip broadcast.
            bool contaminated = false;
            if (m_registry.HasComponent<Velocity>(obstacleEntity) ||
                m_registry.HasComponent<Bullet>(obstacleEntity) ||
                m_registry.HasComponent<Shooter>(obstacleEntity) ||
                m_registry.HasComponent<WeaponSlot>(obstacleEntity)) {
                contaminated = true;
            }
            if (contaminated) {
                std::cerr << "[SERVER OBSTACLE SCRUB] Entity " << obstacleEntity
                          << " had invalid components (Velocity/Bullet/Shooter/WeaponSlot); "
                          << "removing Obstacle/ObstacleMetadata to prevent desync." << std::endl;
                if (m_registry.HasComponent<Obstacle>(obstacleEntity)) {
                    m_registry.RemoveComponent<Obstacle>(obstacleEntity);
                }
                if (m_registry.HasComponent<ObstacleMetadata>(obstacleEntity)) {
                    m_registry.RemoveComponent<ObstacleMetadata>(obstacleEntity);
                }
                // Also remove velocity if present so it stops moving.
                if (m_registry.HasComponent<Velocity>(obstacleEntity)) {
                    m_registry.RemoveComponent<Velocity>(obstacleEntity);
                }
                continue;
            }

            // CRITICAL FIX: If an obstacle somehow has Bullet or Shooter components, strip obstacle data
            // so it can't be broadcast as an obstacle. This prevents “obstacle bullets” even if
            // contamination occurs elsewhere.
            if (m_registry.HasComponent<Bullet>(obstacleEntity) ||
                m_registry.HasComponent<Shooter>(obstacleEntity) ||
                m_registry.HasComponent<WeaponSlot>(obstacleEntity)) {
                std::cerr << "[SERVER BUG] Entity " << obstacleEntity
                          << " has Obstacle + Bullet/Shooter; removing Obstacle to avoid desync." << std::endl;
                if (m_registry.HasComponent<Obstacle>(obstacleEntity)) {
                    m_registry.RemoveComponent<Obstacle>(obstacleEntity);
                }
                if (m_registry.HasComponent<ObstacleMetadata>(obstacleEntity)) {
                    m_registry.RemoveComponent<ObstacleMetadata>(obstacleEntity);
                }
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(obstacleEntity);

            // DEBUG: Log all obstacles being broadcast to help identify the bug
            static int broadcastLog = 0;
            if (broadcastLog < 10 || (broadcastLog % 100 == 0)) {
                std::cerr << "[OBSTACLE BROADCAST] Entity " << obstacleEntity
                          << " pos=(" << pos.x << "," << pos.y << ")"
                          << " hasVelocity=" << m_registry.HasComponent<Velocity>(obstacleEntity)
                          << " hasBullet=" << m_registry.HasComponent<Bullet>(obstacleEntity) << std::endl;
                broadcastLog++;
            }

            // Debug: Log first few obstacle positions sent to clients
            if (serverObstacleLog < 5) {
                std::cout << "[SERVER SEND] Obstacle " << obstacleEntity
                          << " sending pos=(" << pos.x << "," << pos.y << ")";
                if (m_registry.HasComponent<RType::ECS::ObstacleMetadata>(obstacleEntity)) {
                    const auto& meta = m_registry.GetComponent<RType::ECS::ObstacleMetadata>(obstacleEntity);
                    std::cout << " uniqueId=" << meta.uniqueId
                              << " visualEntity=" << meta.visualEntity
                              << " offset=(" << meta.offsetX << "," << meta.offsetY << ")";
                }
                std::cout << std::endl;
                serverObstacleLog++;
            }

            GameEntity entity;
            entity.id = GetOrAssignNetworkId(obstacleEntity);
            entity.type = EntityType::OBSTACLE;
            RegisterNetworkType(entity.id, entity.type);
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
            entity.score = 0;

            m_entities.push_back(entity);
            obstacleCount++;
            if (obstacleCount >= maxObstaclesPerSnapshot) {
                break;
            }
        }

        static bool loggedObstacleSend = false;
        if (!loggedObstacleSend) {
            std::cout << "[SERVER OBSTACLE SYNC] Added " << obstacleCount << " obstacles to network snapshot" << std::endl;
            loggedObstacleSend = true;
        }

        // Sync powerups
        auto powerups = m_registry.GetEntitiesWithComponent<PowerUp>();
        for (auto powerupEntity : powerups) {
            if (!m_registry.IsEntityAlive(powerupEntity) ||
                !m_registry.HasComponent<Position>(powerupEntity) ||
                !m_registry.HasComponent<Velocity>(powerupEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(powerupEntity);
            const auto& vel = m_registry.GetComponent<Velocity>(powerupEntity);
            const auto& powerup = m_registry.GetComponent<PowerUp>(powerupEntity);

            GameEntity entity;
            entity.id = GetOrAssignNetworkId(powerupEntity);
            entity.type = EntityType::POWERUP;
            RegisterNetworkType(entity.id, entity.type);
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vel.dx;
            entity.vy = vel.dy;
            entity.health = 1; // Powerups are "alive" until collected
            entity.flags = static_cast<uint8_t>(powerup.type); // Powerup type
            entity.ownerHash = 0;
            m_entities.push_back(entity);
        }

        // Sync force pods (as separate entities so clients can render them)
        auto forcePods = m_registry.GetEntitiesWithComponent<ForcePod>();
        for (auto podEntity : forcePods) {
            if (!m_registry.IsEntityAlive(podEntity) ||
                !m_registry.HasComponent<Position>(podEntity)) {
                continue;
            }

            const auto& pos = m_registry.GetComponent<Position>(podEntity);
            const auto& pod = m_registry.GetComponent<ForcePod>(podEntity);

            // Find owner to get player hash
            uint64_t ownerHash = 0;
            if (m_registry.IsEntityAlive(pod.owner) &&
                m_registry.HasComponent<Player>(pod.owner)) {
                const auto& owner = m_registry.GetComponent<Player>(pod.owner);
                ownerHash = owner.playerHash;
            }

            // Force pods have velocity from movement system
            float vx = 0.0f, vy = 0.0f;
            if (m_registry.HasComponent<Velocity>(podEntity)) {
                const auto& vel = m_registry.GetComponent<Velocity>(podEntity);
                vx = vel.dx;
                vy = vel.dy;
            }

            GameEntity entity;
            entity.id = GetOrAssignNetworkId(podEntity);
            entity.type = EntityType::PLAYER; // Treat as player entity for rendering
            RegisterNetworkType(entity.id, entity.type);
            entity.x = pos.x;
            entity.y = pos.y;
            entity.vx = vx;
            entity.vy = vy;
            entity.health = 1;
            entity.flags = 0x80; // Bit 7 set to indicate force pod (will need special handling on client)
            entity.ownerHash = ownerHash;
            m_entities.push_back(entity);
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

    bool GameServer::IsBossActive() const {
        auto bosses = m_registry.GetEntitiesWithComponent<RType::ECS::Boss>();

        static bool lastState = false;
        bool currentState = false;

        for (auto bossEntity : bosses) {
            if (!m_registry.IsEntityAlive(bossEntity)) {
                continue;
            }

            if (m_registry.HasComponent<RType::ECS::Health>(bossEntity) &&
                !m_registry.HasComponent<RType::ECS::Scrollable>(bossEntity)) {
                const auto& health = m_registry.GetComponent<RType::ECS::Health>(bossEntity);
                if (health.current > 0) {
                    currentState = true;
                    break;
                }
            }
        }

        if (currentState != lastState) {
            if (currentState) {
                std::cout << "[GameServer] Boss is now ACTIVE - enemy spawning DISABLED" << std::endl;
            } else {
                std::cout << "[GameServer] Boss is DEFEATED - enemy spawning RESUMED" << std::endl;
            }
            lastState = currentState;
        }

        return currentState;
    }

    void GameServer::CheckBossDefeated() {
        if (m_bossDefeated) {
            return;
        }

        auto killedBosses = m_registry.GetEntitiesWithComponent<RType::ECS::BossKilled>();

        if (!killedBosses.empty() && !m_waitingForLevelTransition) {
            m_bossDefeated = true;
            m_levelComplete = true;
            m_waitingForLevelTransition = true;
            m_levelTransitionTimer = 0.0f;

            std::cout << "[GameServer] Boss defeated! Level complete - broadcasting to clients" << std::endl;

            for (auto bossEntity : killedBosses) {
                std::cout << "[GameServer] Destroying boss entity " << bossEntity << std::endl;
                m_registry.DestroyEntity(bossEntity);
            }

            LevelCompletePacket levelComplete;
            levelComplete.completedLevel = m_currentLevel;
            levelComplete.nextLevel = m_currentLevel + 1;

            std::vector<uint8_t> data(sizeof(LevelCompletePacket));
            std::memcpy(data.data(), &levelComplete, sizeof(LevelCompletePacket));
            Broadcast(data);
        }
    }

    void GameServer::LoadNextLevelIfNeeded() {
        if (m_waitingForLevelTransition) {
            m_levelTransitionTimer += (1.0f / 60.0f);

            const float TRANSITION_DELAY = 0.1f;
            if (m_levelTransitionTimer >= TRANSITION_DELAY) {
                std::cout << "[GameServer] Transition delay complete - loading next level" << std::endl;
                m_waitingForLevelTransition = false;

                m_currentLevel++;
                std::string nextLevelPath = "assets/levels/level" + std::to_string(m_currentLevel) + ".json";

                std::cout << "[GameServer] Loading next level: " << nextLevelPath << std::endl;
                std::cout << "[GameServer] Entity count before cleanup: " << m_registry.GetEntityCount() << std::endl;

                std::vector<RType::ECS::Entity> toDestroy;

                auto positionEntities = m_registry.GetEntitiesWithComponent<RType::ECS::Position>();
                std::cout << "[GameServer] Found " << positionEntities.size() << " entities with Position" << std::endl;

                auto colliderEntities = m_registry.GetEntitiesWithComponent<RType::ECS::BoxCollider>();
                std::cout << "[GameServer] Found " << colliderEntities.size() << " entities with BoxCollider" << std::endl;

                auto drawableEntities = m_registry.GetEntitiesWithComponent<RType::ECS::Drawable>();
                std::cout << "[GameServer] Found " << drawableEntities.size() << " entities with Drawable" << std::endl;

                std::unordered_set<RType::ECS::Entity> entitySet;

                for (auto entity : positionEntities) {
                    if (!m_registry.HasComponent<RType::ECS::Player>(entity)) {
                        entitySet.insert(entity);
                    }
                }

                for (auto entity : colliderEntities) {
                    if (!m_registry.HasComponent<RType::ECS::Player>(entity)) {
                        entitySet.insert(entity);
                    }
                }

                for (auto entity : drawableEntities) {
                    if (!m_registry.HasComponent<RType::ECS::Player>(entity)) {
                        entitySet.insert(entity);
                    }
                }

                toDestroy.assign(entitySet.begin(), entitySet.end());

                std::cout << "[GameServer] Total unique entities to destroy: " << toDestroy.size() << std::endl;

                auto playerEntitiesVec = m_registry.GetEntitiesWithComponent<RType::ECS::Player>();
                std::cout << "[GameServer] Preserving " << playerEntitiesVec.size() << " player entities:" << std::endl;
                for (auto entity : playerEntitiesVec) {
                    std::cout << "  - Player entity " << entity << std::endl;
                }

                for (RType::ECS::Entity entity : toDestroy) {
                    m_registry.DestroyEntity(entity);
                }

                std::cout << "[GameServer] Destroyed " << toDestroy.size() << " non-player entities (preserved " << playerEntitiesVec.size() << " players)" << std::endl;
                std::cout << "[GameServer] Entity count after cleanup: " << m_registry.GetEntityCount() << std::endl;

                std::vector<GameEntity> playerEntities;
                for (const auto& ge : m_entities) {
                    if (ge.type == EntityType::PLAYER) {
                        playerEntities.push_back(ge);
                    }
                }
                m_entities = playerEntities;

                std::cout << "[GameServer] Kept " << m_entities.size() << " player GameEntity entries" << std::endl;

                // Reset level state
                m_bossDefeated = false;
                m_levelComplete = false;
                m_scrollOffset = 0.0f;
                m_enemyShootCooldowns.clear();
                m_enemyBulletTypes.clear();

                // Load new level
                m_levelPath = nextLevelPath;
                try {
                    auto levelData = RType::ECS::LevelLoader::LoadFromFile(m_levelPath);
                    std::cout << "[GameServer] Level " << m_currentLevel << " loaded" << std::endl;

                    auto createdEntities = RType::ECS::LevelLoader::CreateServerEntities(m_registry, levelData);
                    std::cout << "[GameServer] Level entities created: "
                              << createdEntities.obstacleColliders.size() << " obstacles, "
                              << createdEntities.enemies.size() << " enemies, "
                              << "boss: " << (createdEntities.boss != RType::ECS::NULL_ENTITY ? "YES" : "NO")
                              << std::endl;

                    auto playerEntities = m_registry.GetEntitiesWithComponent<RType::ECS::Player>();
                    std::cout << "[GameServer] Repositioning " << playerEntities.size() << " players to spawn points" << std::endl;

                    size_t spawnIndex = 0;
                    for (auto playerEntity : playerEntities) {
                        if (spawnIndex < levelData.playerSpawns.size()) {
                            const auto& spawn = levelData.playerSpawns[spawnIndex];

                            if (m_registry.HasComponent<RType::ECS::Position>(playerEntity)) {
                                auto& pos = m_registry.GetComponent<RType::ECS::Position>(playerEntity);
                                pos.x = spawn.x;
                                pos.y = spawn.y;
                                std::cout << "[GameServer] Player " << playerEntity << " repositioned to ("
                                          << spawn.x << ", " << spawn.y << ")" << std::endl;
                            }

                            if (m_registry.HasComponent<RType::ECS::Health>(playerEntity)) {
                                auto& health = m_registry.GetComponent<RType::ECS::Health>(playerEntity);
                                health.current = health.max;
                                std::cout << "[GameServer] Player " << playerEntity << " health reset to "
                                          << health.max << std::endl;
                            }

                            spawnIndex++;
                        }
                    }

                } catch (const std::exception& e) {
                    std::cerr << "[GameServer] Failed to load level " << m_currentLevel << ": " << e.what() << std::endl;
                }
            }
        }
    }

}
