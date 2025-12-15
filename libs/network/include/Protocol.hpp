/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Protocol
*/

#pragma once

#include <cstdint>
#include <cstddef>

// ==================== CONSTANTS ====================
constexpr size_t PLAYER_NAME_SIZE = 32;
constexpr size_t MAX_PLAYERS = 4;
constexpr size_t MAX_ENTITIES = 256;

namespace network {

    // ==================== LOBBY PROTOCOL (TCP) ====================
    enum class LobbyPacket : uint8_t {
        CONNECT_REQ = 0x10,
        CONNECT_ACK = 0x11,
        PLAYER_JOIN = 0x12,
        READY_REQ = 0x13,
        PLAYER_READY = 0x14,
        START_REQ = 0x15,
        GAME_START = 0x16,
        DISCONNECT = 0x17,
        PLAYER_LEFT = 0x18,
        COUNTDOWN = 0x19,
        ERROR_MSG = 0x1F, // @haloys i renamed to avoid Windows macro conflict
    };

    // ==================== GAME PROTOCOL (UDP) ====================
    enum class GamePacket : uint8_t {
        HELLO = 0x00,      // Client → Server: Initial connection
        WELCOME = 0x01,    // Server → Client: Connection accepted
        INPUT = 0x02,      // Client → Server: Player inputs
        STATE = 0x03,      // Server → Client: Game state snapshot
        PING = 0x04,       // Bidirectional: Keepalive
        PONG = 0x05,       // Bidirectional: Keepalive response
        DISCONNECT = 0x06, // Client → Server: Graceful disconnect
    };

    // Input flags bitfield
    enum InputFlags : uint8_t {
        NONE = 0,
        UP = 1 << 0,    // 0x01
        DOWN = 1 << 1,  // 0x02
        LEFT = 1 << 2,  // 0x04
        RIGHT = 1 << 3, // 0x08
        SHOOT = 1 << 4, // 0x10
    };

    // Entity types for state packets
    enum class EntityType : uint8_t {
        PLAYER = 0x01,
        ENEMY = 0x02,
        BULLET = 0x03,
        POWERUP = 0x04,
        OBSTACLE = 0x05,
    };

    // ==================== STRUCTURES ====================
    struct PlayerInfo {
        uint8_t number = 0;
        uint64_t hash = 0;
        char name[PLAYER_NAME_SIZE] = {};
        bool ready = false;
    };

    // Game start information (sent via TCP)
    struct GameStartInfo {
        char serverIp[16] = "127.0.0.1";
        uint16_t udpPort = 0;
        uint8_t playerCount = 0;
        PlayerInfo players[MAX_PLAYERS];
        float spawnPositions[MAX_PLAYERS][2]; // [x, y]
    };

// UDP Packet structures
#pragma pack(push, 1)

    // HELLO packet (Client → Server)
    struct HelloPacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::HELLO);
        uint64_t playerHash = 0;
        char playerName[PLAYER_NAME_SIZE] = {};
    };

    // WELCOME packet (Server → Client)
    struct WelcomePacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::WELCOME);
        uint8_t playersConnected = 0;
        uint32_t serverTick = 0;
    };

    // INPUT packet (Client → Server)
    struct InputPacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::INPUT);
        uint32_t sequence = 0;   // Client sequence number
        uint64_t playerHash = 0; // Player identifier
        uint8_t inputs = 0;      // Bitfield of InputFlags
        uint32_t timestamp = 0;  // Client timestamp (ms)
    };

    // Entity state in STATE packet
    struct EntityState {
        uint32_t entityId = 0;
        uint8_t entityType = 0; // EntityType
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f; // Velocity X
        float vy = 0.0f; // Velocity Y
        uint8_t health = 0;
        uint8_t flags = 0;      // Custom flags per entity type
        uint64_t ownerHash = 0; // Player hash for PLAYER entities (for client-side prediction)
        uint32_t score = 0;
    };

    // STATE packet (Server → Client)
    struct StatePacketHeader {
        uint8_t type = static_cast<uint8_t>(GamePacket::STATE);
        uint32_t tick = 0;         // Server tick number
        uint32_t timestamp = 0;    // Server timestamp (ms)
        uint16_t entityCount = 0;  // Number of entities following
        float scrollOffset = 0.0f; // Background scroll offset
    };
    // Followed by: EntityState[entityCount]

    // PING packet
    struct PingPacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::PING);
        uint32_t timestamp = 0;
    };

    // PONG packet
    struct PongPacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::PONG);
        uint32_t timestamp = 0; // Echo of PING timestamp
    };

#pragma pack(pop)

}
