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
constexpr size_t ROOM_NAME_SIZE = 32;
constexpr size_t MAX_PLAYERS = 4;
constexpr size_t MAX_ROOMS = 16;
constexpr size_t MAX_ENTITIES = 256;

namespace network {

    // ==================== LOBBY PROTOCOL (TCP) ====================
    enum class LobbyPacket : uint8_t {
        // Room management packets
        LIST_ROOMS_REQ = 0x01,   // Client → Server: request room list
        LIST_ROOMS_ACK = 0x02,   // Server → Client: room list response
        CREATE_ROOM_REQ = 0x03,  // Client → Server: create a new room
        CREATE_ROOM_ACK = 0x04,  // Server → Client: room created (with ID)
        JOIN_ROOM_REQ = 0x05,    // Client → Server: join a room
        JOIN_ROOM_ACK = 0x06,    // Server → Client: join accepted/rejected
        ROOM_UPDATE = 0x07,      // Server → All: room state changed

        // Lobby packets (within a room)
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
        ERROR_MSG = 0x1F,
    };

    // ==================== GAME PROTOCOL (UDP) ====================
    enum class GamePacket : uint8_t {
        HELLO = 0x00,      // Client → Server: Initial connection
        WELCOME = 0x01,    // Server → Client: Connection accepted
        INPUT = 0x02,      // Client → Server: Player inputs
        STATE = 0x03,      // Server → Client: Game state snapshot (full)
        PING = 0x04,       // Bidirectional: Keepalive
        PONG = 0x05,       // Bidirectional: Keepalive response
        DISCONNECT = 0x06, // Client → Server: Graceful disconnect
        LEVEL_COMPLETE = 0x07, // Server → Client: Level completed (boss defeated)
        STATE_DELTA = 0x08,    // Server → Client: Delta state snapshot (optimized)
        STATE_ACK = 0x09,      // Client → Server: Acknowledge received state sequence
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
        BOSS = 0x06,
    };

    // ==================== STRUCTURES ====================

    // Room information for room list
    struct RoomInfo {
        uint32_t id = 0;
        char name[ROOM_NAME_SIZE] = {};
        uint8_t playerCount = 0;
        uint8_t maxPlayers = MAX_PLAYERS;
        bool inGame = false;
    };

    // Join room result status
    enum class JoinRoomStatus : uint8_t {
        SUCCESS = 0x00,
        ROOM_FULL = 0x01,
        ROOM_NOT_FOUND = 0x02,
        ROOM_IN_GAME = 0x03,
    };

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

    // Power-up flags for EntityState (bitfield)
    enum PowerUpFlags : uint8_t {
        POWERUP_NONE = 0,
        POWERUP_FIRE_RATE_BOOST = 1 << 0,  // 0x01
        POWERUP_SPREAD_SHOT = 1 << 1,       // 0x02
        POWERUP_LASER_BEAM = 1 << 2,        // 0x04
        POWERUP_SHIELD = 1 << 3,            // 0x08
        POWERUP_FORCE_POD = 1 << 4,         // 0x10
    };

    // Entity state in STATE packet
    struct EntityState {
        uint32_t entityId = 0;
        uint8_t entityType = 0; // EntityType
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f; // Velocity X
        float vy = 0.0f; // Velocity Y
        uint16_t health = 0;    // Changed to uint16_t to support 300 HP
        uint8_t flags = 0;      // Custom flags per entity type
        uint64_t ownerHash = 0; // Player hash for PLAYER entities (for client-side prediction)
        uint32_t score = 0;
        uint8_t powerUpFlags = 0;
        uint8_t speedMultiplier = 10;
        uint8_t weaponType = 0;
        uint8_t fireRate = 20;
    };

    // Input acknowledgment per player (sent with STATE packet)
    struct InputAck {
        uint64_t playerHash = 0;
        uint32_t lastProcessedSeq = 0;
        float serverPosX = 0.0f;
        float serverPosY = 0.0f;
    };

    // STATE packet (Server → Client) - Full snapshot
    struct StatePacketHeader {
        uint8_t type = static_cast<uint8_t>(GamePacket::STATE);
        uint32_t tick = 0;         // Server tick number
        uint32_t timestamp = 0;    // Server timestamp (ms)
        uint16_t entityCount = 0;  // Number of entities following
        float scrollOffset = 0.0f; // Background scroll offset
        uint8_t inputAckCount = 0;
        uint32_t stateSequence = 0; // Sequence number for delta tracking
    };

    // Delta update flags - which fields changed
    enum DeltaFlags : uint8_t {
        DELTA_POSITION = 1 << 0,    // x, y changed
        DELTA_VELOCITY = 1 << 1,    // vx, vy changed
        DELTA_HEALTH = 1 << 2,      // health changed
        DELTA_FLAGS = 1 << 3,       // flags changed
        DELTA_SCORE = 1 << 4,       // score changed
        DELTA_POWERUP = 1 << 5,     // powerUpFlags changed
        DELTA_WEAPON = 1 << 6,      // weaponType/fireRate changed
        DELTA_DESTROYED = 1 << 7,   // Entity was destroyed
    };

    // Delta entity state - only sends changed fields
    struct DeltaEntityHeader {
        uint32_t entityId = 0;
        uint8_t deltaFlags = 0;     // Which fields are included
    };

    // STATE_DELTA packet header (Server → Client) - Delta snapshot
    struct StateDeltaHeader {
        uint8_t type = static_cast<uint8_t>(GamePacket::STATE_DELTA);
        uint32_t tick = 0;              // Server tick number
        uint32_t timestamp = 0;         // Server timestamp (ms)
        uint32_t stateSequence = 0;     // Current sequence number
        uint32_t baseSequence = 0;      // Base sequence this delta is from
        uint16_t deltaEntityCount = 0;  // Number of changed entities
        uint16_t destroyedCount = 0;    // Number of destroyed entity IDs
        uint16_t newEntityCount = 0;    // Number of new full entities
        float scrollOffset = 0.0f;      // Background scroll offset
        uint8_t inputAckCount = 0;
        uint8_t compressionFlags = 0;   // 0 = none, 1 = LZ4
        uint32_t uncompressedSize = 0;  // Original size before compression (0 if not compressed)
    };

    // STATE_ACK packet (Client → Server) - Acknowledge received state
    struct StateAckPacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::STATE_ACK);
        uint64_t playerHash = 0;
        uint32_t lastReceivedSeq = 0;   // Last successfully received state sequence
    };

    // Compression flags
    enum CompressionFlags : uint8_t {
        COMPRESSION_NONE = 0x00,
        COMPRESSION_LZ4 = 0x01,
    };

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

    // LEVEL_COMPLETE packet (Server → Client)
    struct LevelCompletePacket {
        uint8_t type = static_cast<uint8_t>(GamePacket::LEVEL_COMPLETE);
        uint8_t completedLevel = 0;  // Level number that was completed
        uint8_t nextLevel = 0;        // Next level to load (0 = no more levels)
    };

#pragma pack(pop)

}
