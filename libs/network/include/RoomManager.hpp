/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomManager - Manages multiple game rooms
*/

#pragma once

#include "TcpServer.hpp"
#include "TcpSocket.hpp"
#include "Protocol.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include <vector>
#include <unordered_map>
#include <random>
#include <optional>
#include <memory>
#include <chrono>
#include <string>
#include <functional>

namespace network {

    class RoomManager {
    public:
        struct Room {
            uint32_t id = 0;
            std::string name;
            std::vector<std::unique_ptr<TcpSocket>> clients;
            std::vector<std::optional<PlayerInfo>> players;
            bool inGame = false;
            bool countdownActive = false;
            float countdownTimer = 5.0f;
            std::chrono::steady_clock::time_point lastUpdateTime;
            int lastBroadcastedSecond = -1;
        };

        RoomManager(uint16_t port, size_t maxRooms = MAX_ROOMS, size_t minPlayersPerRoom = 2);

        void update();
        void printStatus() const;

        const std::unordered_map<uint32_t, Room>& getRooms() const { return _rooms; }
        size_t roomCount() const { return _rooms.size(); }

        using GameStartCallback = std::function<void(uint32_t roomId, const Room& room)>;
        void onGameStart(GameStartCallback callback) { _onGameStart = callback; }

    private:
        // Connection management
        void acceptNewClients();
        void processClients();

        // Packet handlers
        void handlePacket(TcpSocket& client, const std::vector<uint8_t>& data);
        void handleListRooms(TcpSocket& client);
        void handleCreateRoom(TcpSocket& client, Deserializer& d);
        void handleJoinRoom(TcpSocket& client, Deserializer& d);

        // Room-specific packet handlers
        void handleConnect(Room& room, size_t clientIdx, Deserializer& d);
        void handleReady(Room& room, size_t clientIdx, Deserializer& d);
        void handleStart(Room& room, size_t clientIdx);
        void handleDisconnect(Room& room, size_t clientIdx);

        uint32_t createRoom(const std::string& name);
        JoinRoomStatus joinRoom(uint32_t roomId, TcpSocket& client);
        void removeClientFromRoom(Room& room, size_t idx);
        void cleanupEmptyRooms();
        void updateRoomCountdown(Room& room);
        bool isRoomReady(const Room& room) const;
        size_t findFreeSlotInRoom(const Room& room) const;
        size_t activePlayerCountInRoom(const Room& room) const;

        void sendTo(TcpSocket& client, LobbyPacket type, const std::vector<uint8_t>& payload = {});
        void sendToRoom(Room& room, size_t clientIdx, LobbyPacket type, const std::vector<uint8_t>& payload = {});
        void broadcastToRoom(Room& room, LobbyPacket type, const std::vector<uint8_t>& payload = {});
        void broadcastToRoomExcept(Room& room, size_t exceptIdx, LobbyPacket type, const std::vector<uint8_t>& payload = {});
        void broadcastRoomUpdate(uint32_t roomId);
        uint64_t generateHash();
        RoomInfo getRoomInfo(const Room& room, uint32_t roomId) const;

        std::pair<uint32_t, size_t> findClientRoom(TcpSocket* clientPtr);

        TcpServer _server;
        std::unordered_map<uint32_t, Room> _rooms;
        std::vector<std::unique_ptr<TcpSocket>> _pendingClients;
        size_t _maxRooms;
        size_t _minPlayersPerRoom;
        uint32_t _nextRoomId = 1;
        std::mt19937_64 _rng;

        GameStartCallback _onGameStart;
    };

}
