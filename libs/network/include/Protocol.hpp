/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Protocol
*/

#pragma once

#include <cstdint>

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
        INPUT = 0x01,
        STATE = 0x02,
        ACK = 0x03,
        PING = 0x04,
        PONG = 0x05,
    };

    // ==================== CONSTANTS ====================
    constexpr size_t PLAYER_NAME_SIZE = 32;
    constexpr size_t MAX_PLAYERS = 4;

    // ==================== STRUCTURES ====================
    struct PlayerInfo {
        uint8_t number = 0;
        uint64_t hash = 0;
        char name[PLAYER_NAME_SIZE] = {};
        bool ready = false;
    };

}
