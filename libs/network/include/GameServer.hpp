/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameServer
*/

#pragma once

#include "Protocol.hpp"
#include "Endpoint.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include "ECS/MovementSystem.hpp"
#include "ECS/EnemySystem.hpp"
#include "ECS/CollisionDetectionSystem.hpp"
#include "ECS/BulletCollisionResponseSystem.hpp"
#include "ECS/PlayerCollisionResponseSystem.hpp"
#include "ECS/ObstacleCollisionResponseSystem.hpp"
#include "ECS/ScrollingSystem.hpp"
#include "ECS/BossSystem.hpp"
#include "ECS/BossAttackSystem.hpp"
#include "ECS/BlackOrbSystem.hpp"
#include "ECS/ThirdBulletSystem.hpp"
#include "ECS/LevelLoader.hpp"
#include "ECS/HealthSystem.hpp"
#include "ECS/ScoreSystem.hpp"
#include "ECS/PlayerFactory.hpp"
#include "ECS/EnemyFactory.hpp"
#include "ECS/PowerUpSpawnSystem.hpp"
#include "ECS/PowerUpCollisionSystem.hpp"
#include "ECS/ShootingSystem.hpp"
#include "ECS/ForcePodSystem.hpp"
#include "ECS/ShieldSystem.hpp"
#include <asio.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <cmath>
#include <array>
#include <string>

namespace network {

    enum class EnemyType : uint8_t {
        BASIC = 0,
        FAST = 1,
        TANK = 2,
        BOSS = 3,
        FORMATION = 4
    };

    struct GameEntity;

    using MovementPatternFunc = void (*)(GameEntity&, float);

    struct EnemyStats {
        float speed;
        uint8_t health;
        uint8_t damage;
        float fireRate;
        float bulletXOffset;
        float bulletYOffset;
        uint8_t collisionDamageMultiplier;
        MovementPatternFunc movementPattern;
    };

    struct ConnectedPlayer {
        PlayerInfo info;
        Endpoint endpoint;
        std::chrono::steady_clock::time_point lastPingTime;
        uint32_t lastInputSequence = 0;
        bool alive = true;
    };

    struct GameEntity {
        uint32_t id;
        EntityType type;
        float x, y;
        float vx, vy;
        uint8_t health;
        uint8_t flags;
        uint64_t ownerHash = 0;
        uint32_t score = 0;
        uint8_t powerUpFlags = 0;
        uint8_t speedMultiplier = 10;
        uint8_t weaponType = 0;
        uint8_t fireRate = 20;
    };

    class GameServer {
    public:
        GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers, const std::string& levelPath = "assets/levels/level1.json");
        ~GameServer();

        void Run();
        void Stop();
        bool AllPlayersDisconnected() const;

        uint32_t GetCurrentTick() const { return m_currentTick; }
        size_t GetConnectedPlayerCount() const { return m_connectedPlayers.size(); }
        uint64_t GetPacketsSent() const { return m_packetsSent; }
        uint64_t GetPacketsReceived() const { return m_packetsReceived; }
        const std::string& GetLevelPath() const { return m_levelPath; }
    private:
        void WaitForAllPlayers();
        void ProcessIncomingPackets();
        void SendStateSnapshots();
        void HandlePacket(const std::vector<uint8_t>& data, const Endpoint& from);
        void HandleHello(const std::vector<uint8_t>& data, const Endpoint& from);
        void HandleInput(const std::vector<uint8_t>& data, const Endpoint& from);
        void HandlePing(const std::vector<uint8_t>& data, const Endpoint& from);

        void SendTo(const std::vector<uint8_t>& data, const Endpoint& to);
        void Broadcast(const std::vector<uint8_t>& data);

        void UpdateGameLogic(float dt);
        void SpawnPlayer(uint64_t hash, float x, float y);
        void SpawnEnemy();
        bool IsBossActive() const;
        void SpawnEnemyBullet(uint32_t enemyId, float x, float y, uint8_t enemyType);
        void SpawnBullet(uint64_t ownerHash, float x, float y);
        void UpdateBullets(float dt);
        void UpdateEnemies(float dt);
        void CleanupDeadEntities();
        void UpdateLegacyEntitiesFromRegistry();

        EnemyType GetRandomEnemyType();
        const EnemyStats& GetEnemyStats(EnemyType type) const;

        uint32_t GetNextEntityId() { return m_nextEntityId++; }

        asio::io_context m_ioContext;
        asio::ip::udp::socket m_socket;
        std::vector<PlayerInfo> m_expectedPlayers;
        std::unordered_map<uint64_t, ConnectedPlayer> m_connectedPlayers;
        const std::chrono::seconds DISCONNECT_TIMEOUT{10};

        RType::ECS::Registry m_registry;
        std::unique_ptr<RType::ECS::ScrollingSystem> m_scrollingSystem;
        std::unique_ptr<RType::ECS::BossSystem> m_bossSystem;
        std::unique_ptr<RType::ECS::BossAttackSystem> m_bossAttackSystem;
        std::unique_ptr<RType::ECS::BlackOrbSystem> m_blackOrbSystem;
        std::unique_ptr<RType::ECS::ThirdBulletSystem> m_thirdBulletSystem;
        std::unique_ptr<RType::ECS::MovementSystem> m_movementSystem;
        std::unique_ptr<RType::ECS::CollisionDetectionSystem> m_collisionDetectionSystem;
        std::unique_ptr<RType::ECS::BulletCollisionResponseSystem> m_bulletResponseSystem;
        std::unique_ptr<RType::ECS::PlayerCollisionResponseSystem> m_playerResponseSystem;
        std::unique_ptr<RType::ECS::ObstacleCollisionResponseSystem> m_obstacleResponseSystem;
        std::unique_ptr<RType::ECS::HealthSystem> m_healthSystem;
        std::unique_ptr<RType::ECS::ScoreSystem> m_scoreSystem;
        std::unique_ptr<RType::ECS::PowerUpSpawnSystem> m_powerUpSpawnSystem;
        std::unique_ptr<RType::ECS::PowerUpCollisionSystem> m_powerUpCollisionSystem;
        std::unique_ptr<RType::ECS::ShootingSystem> m_shootingSystem;
        std::unique_ptr<RType::ECS::ForcePodSystem> m_forcePodSystem;
        std::unique_ptr<RType::ECS::ShieldSystem> m_shieldSystem;

        std::vector<GameEntity> m_entities;
        uint32_t m_currentTick = 0;
        uint32_t m_nextEntityId = 1;
        std::atomic<bool> m_running{false};

        float m_scrollOffset = 0.0f;
        const float SCROLL_SPEED = -150.0f;

        std::chrono::steady_clock::time_point m_lastSpawnTime;
        float m_enemySpawnInterval = 2.0f;

        std::unordered_map<uint32_t, float> m_enemyShootCooldowns;
        std::unordered_map<uint32_t, uint8_t> m_enemyBulletTypes;

        std::atomic<uint64_t> m_packetsSent{0};
        std::atomic<uint64_t> m_packetsReceived{0};

        static const std::array<EnemyStats, 5> s_enemyStats;

        std::string m_levelPath;
    };

}
