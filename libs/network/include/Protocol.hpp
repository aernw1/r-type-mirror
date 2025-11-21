#pragma once

#include <cstdint>

namespace network {

    enum class PacketType : uint8_t {
        CONNECT_REQ = 0x10,
        CONNECT_ACK = 0x11,
        PLAYER_JOIN = 0x12,
        READY_REQ = 0x15,
        PLAYER_READY = 0x16,
        START_REQ = 0x17,
        GAME_START = 0x18,
        DISCONNECT = 0x19,
        ERROR = 0xFF,

        INPUT = 0x01,
        STATE = 0x02,
        ACK = 0x03,
    };

    constexpr size_t PLAYER_NAME_SIZE = 32;
    constexpr size_t MAX_PLAYERS = 4;

    struct PlayerInfo {
        uint8_t number = 0;
        uint64_t hash = 0;
        char name[PLAYER_NAME_SIZE] = {};
        bool ready = false;
    };

}
