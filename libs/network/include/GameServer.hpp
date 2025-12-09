/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameServer
*/

#pragma once

#include "Protocol.hpp"
#include "Endpoint.hpp"
#include <asio.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>

namespace network {

    enum class EnemyType : uint8_t {
        BASIC = 0,
        FAST = 1,
        TANK = 2,
        BOSS = 3,
        FORMATION = 4
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
    };

    class GameServer {
    public:
        GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers);
        ~GameServer();

        void Run();
        void Stop();

        uint32_t GetCurrentTick() const { return m_currentTick; }
        size_t GetConnectedPlayerCount() const { return m_connectedPlayers.size(); }
        uint64_t GetPacketsSent() const { return m_packetsSent; }
        uint64_t GetPacketsReceived() const { return m_packetsReceived; }
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
        void SpawnEnemyBullet(uint32_t enemyId, float x, float y);
        void SpawnBullet(uint64_t ownerHash, float x, float y);
        void UpdateMovement(float dt);
        void UpdateBullets(float dt);
        void UpdateEnemies(float dt);
        void CheckCollisions();
        void CleanupDeadEntities();

        EnemyType GetRandomEnemyType();
        float GetEnemySpeed(EnemyType type);
        uint8_t GetEnemyHealth(EnemyType type);
        uint8_t GetEnemyDamage(EnemyType type);
        float GetEnemyFireRate(EnemyType type);
        void ApplyEnemyMovementPattern(GameEntity& enemy, float dt);
        bool HasPlayerInSight(const GameEntity& enemy);

        uint32_t GetNextEntityId() { return m_nextEntityId++; }
        GameEntity* FindEntityById(uint32_t id);
        std::vector<GameEntity*> GetEntitiesByType(EntityType type);

        asio::io_context m_ioContext;
        asio::ip::udp::socket m_socket;
        std::vector<PlayerInfo> m_expectedPlayers;
        std::unordered_map<uint64_t, ConnectedPlayer> m_connectedPlayers;

        std::vector<GameEntity> m_entities;
        uint32_t m_currentTick = 0;
        uint32_t m_nextEntityId = 1;
        std::atomic<bool> m_running{false};

        // Map scrolling
        float m_scrollOffset = 0.0f;
        const float SCROLL_SPEED = -150.0f;

        std::chrono::steady_clock::time_point m_lastSpawnTime;
        float m_enemySpawnInterval = 2.0f;

        std::unordered_map<uint32_t, float> m_enemyShootCooldowns;

        std::atomic<uint64_t> m_packetsSent{0};
        std::atomic<uint64_t> m_packetsReceived{0};
    };

}
