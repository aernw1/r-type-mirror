/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomManager
*/

#include "RoomManager.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace network {

    static const char* lobbyPacketName(LobbyPacket type) {
        switch (type) {
        case LobbyPacket::LIST_ROOMS_REQ: return "LIST_ROOMS_REQ";
        case LobbyPacket::LIST_ROOMS_ACK: return "LIST_ROOMS_ACK";
        case LobbyPacket::CREATE_ROOM_REQ: return "CREATE_ROOM_REQ";
        case LobbyPacket::CREATE_ROOM_ACK: return "CREATE_ROOM_ACK";
        case LobbyPacket::JOIN_ROOM_REQ: return "JOIN_ROOM_REQ";
        case LobbyPacket::JOIN_ROOM_ACK: return "JOIN_ROOM_ACK";
        case LobbyPacket::ROOM_UPDATE: return "ROOM_UPDATE";
        case LobbyPacket::CONNECT_REQ: return "CONNECT_REQ";
        case LobbyPacket::CONNECT_ACK: return "CONNECT_ACK";
        case LobbyPacket::PLAYER_JOIN: return "PLAYER_JOIN";
        case LobbyPacket::READY_REQ: return "READY_REQ";
        case LobbyPacket::PLAYER_READY: return "PLAYER_READY";
        case LobbyPacket::START_REQ: return "START_REQ";
        case LobbyPacket::GAME_START: return "GAME_START";
        case LobbyPacket::DISCONNECT: return "DISCONNECT";
        case LobbyPacket::PLAYER_LEFT: return "PLAYER_LEFT";
        case LobbyPacket::COUNTDOWN: return "COUNTDOWN";
        case LobbyPacket::ERROR_MSG: return "ERROR";
        default: return "UNKNOWN";
        }
    }

    RoomManager::RoomManager(uint16_t port, size_t maxRooms, size_t minPlayersPerRoom)
        : _server(port), _maxRooms(maxRooms), _minPlayersPerRoom(minPlayersPerRoom), _rng(std::random_device{}()) {
        std::cout << "[RoomManager] Server started on port " << port << " (maxRooms=" << _maxRooms << ", minPlayers=" << _minPlayersPerRoom << ")" << std::endl;
    }

    void RoomManager::update() {
        acceptNewClients();
        processClients();

        for (auto& [roomId, room] : _rooms) {
            updateRoomCountdown(room);
        }
    }

    void RoomManager::acceptNewClients() {
        while (auto newClient = _server.accept()) {
            std::cout << "[RoomManager] New connection accepted (pending room selection)" << std::endl;
            _pendingClients.push_back(std::make_unique<TcpSocket>(std::move(*newClient)));
        }
    }

    void RoomManager::processClients() {
        for (size_t i = 0; i < _pendingClients.size();) {
            if (!_pendingClients[i]) {
                _pendingClients.erase(_pendingClients.begin() + i);
                continue;
            }
            if (!_pendingClients[i]->isConnected()) {
                std::cout << "[RoomManager] Pending client disconnected" << std::endl;
                _pendingClients.erase(_pendingClients.begin() + i);
                continue;
            }

            size_t sizeBefore = _pendingClients.size();
            TcpSocket* clientPtr = _pendingClients[i].get();

            while (auto data = clientPtr->receive()) {
                handlePacket(*clientPtr, *data);
                if (_pendingClients.size() < sizeBefore) {
                    break;
                }
            }

            if (_pendingClients.size() == sizeBefore) {
                ++i;
            }
        }

        for (auto& [roomId, room] : _rooms) {
            for (size_t idx = 0; idx < MAX_PLAYERS; ++idx) {
                if (!room.clients[idx])
                    continue;
                if (!room.clients[idx]->isConnected()) {
                    removeClientFromRoom(room, idx);
                    broadcastRoomUpdate(roomId);
                    continue;
                }
                while (auto data = room.clients[idx]->receive()) {
                    if (data->empty())
                        continue;
                    Deserializer d(*data);
                    auto type = static_cast<LobbyPacket>(d.readU8());
                    std::cout << "[RoomManager] << RECV " << lobbyPacketName(type) << " from room " << roomId << " client #" << (idx + 1) << std::endl;

                    switch (type) {
                    case LobbyPacket::CONNECT_REQ:
                        handleConnect(room, idx, d);
                        break;
                    case LobbyPacket::READY_REQ:
                        handleReady(room, idx, d);
                        break;
                    case LobbyPacket::START_REQ:
                        handleStart(room, idx);
                        break;
                    case LobbyPacket::DISCONNECT:
                        handleDisconnect(room, idx);
                        broadcastRoomUpdate(roomId);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    void RoomManager::handlePacket(TcpSocket& client, const std::vector<uint8_t>& data) {
        if (data.empty())
            return;

        Deserializer d(data);
        auto type = static_cast<LobbyPacket>(d.readU8());
        std::cout << "[RoomManager] << RECV " << lobbyPacketName(type) << " from pending client" << std::endl;

        switch (type) {
        case LobbyPacket::LIST_ROOMS_REQ:
            handleListRooms(client);
            break;
        case LobbyPacket::CREATE_ROOM_REQ:
            handleCreateRoom(client, d);
            break;
        case LobbyPacket::JOIN_ROOM_REQ:
            handleJoinRoom(client, d);
            break;
        default:
            std::cout << "[RoomManager] Unexpected packet from pending client: " << lobbyPacketName(type) << std::endl;
            break;
        }
    }

    void RoomManager::handleListRooms(TcpSocket& client) {
        Serializer s;
        s.writeU8(static_cast<uint8_t>(_rooms.size()));

        for (const auto& [roomId, room] : _rooms) {
            RoomInfo info = getRoomInfo(room, roomId);
            s.writeU32(info.id);
            s.writeString(info.name, ROOM_NAME_SIZE);
            s.writeU8(info.playerCount);
            s.writeU8(info.maxPlayers);
            s.writeU8(info.inGame ? 1 : 0);
        }

        sendTo(client, LobbyPacket::LIST_ROOMS_ACK, s.finalize());
        std::cout << "[RoomManager] Sent room list (" << _rooms.size() << " rooms)" << std::endl;
    }

    void RoomManager::handleCreateRoom(TcpSocket& client, Deserializer& d) {
        std::string roomName = d.readString(ROOM_NAME_SIZE);

        if (_rooms.size() >= _maxRooms) {
            Serializer s;
            s.writeU8(0x01);
            s.writeU32(0);
            sendTo(client, LobbyPacket::CREATE_ROOM_ACK, s.finalize());
            std::cout << "[RoomManager] Room creation failed: max rooms reached" << std::endl;
            return;
        }

        uint32_t roomId = createRoom(roomName);

        Serializer s;
        s.writeU8(0x00);
        s.writeU32(roomId);
        sendTo(client, LobbyPacket::CREATE_ROOM_ACK, s.finalize());

        std::cout << "[RoomManager] Room '" << roomName << "' created with ID " << roomId << std::endl;
    }

    void RoomManager::handleJoinRoom(TcpSocket& client, Deserializer& d) {
        uint32_t roomId = d.readU32();

        auto it = std::find_if(_pendingClients.begin(), _pendingClients.end(),
            [&client](const std::unique_ptr<TcpSocket>& ptr) {
                return ptr.get() == &client;
            });

        if (it == _pendingClients.end()) {
            std::cout << "[RoomManager] Join room failed: client not found in pending list" << std::endl;
            return;
        }

        auto roomIt = _rooms.find(roomId);
        if (roomIt == _rooms.end()) {
            Serializer s;
            s.writeU8(static_cast<uint8_t>(JoinRoomStatus::ROOM_NOT_FOUND));
            sendTo(client, LobbyPacket::JOIN_ROOM_ACK, s.finalize());
            return;
        }

        Room& room = roomIt->second;

        if (room.inGame) {
            Serializer s;
            s.writeU8(static_cast<uint8_t>(JoinRoomStatus::ROOM_IN_GAME));
            sendTo(client, LobbyPacket::JOIN_ROOM_ACK, s.finalize());
            return;
        }

        size_t slot = findFreeSlotInRoom(room);
        if (slot >= MAX_PLAYERS) {
            Serializer s;
            s.writeU8(static_cast<uint8_t>(JoinRoomStatus::ROOM_FULL));
            sendTo(client, LobbyPacket::JOIN_ROOM_ACK, s.finalize());
            return;
        }

        room.clients[slot] = std::move(*it);
        room.players[slot] = PlayerInfo{};
        _pendingClients.erase(it);

        Serializer s;
        s.writeU8(static_cast<uint8_t>(JoinRoomStatus::SUCCESS));
        sendToRoom(room, slot, LobbyPacket::JOIN_ROOM_ACK, s.finalize());

        std::cout << "[RoomManager] Client joined room " << roomId << " (slot #" << (slot + 1) << ")" << std::endl;

        broadcastRoomUpdate(roomId);
    }

    void RoomManager::handleConnect(Room& room, size_t clientIdx, Deserializer& d) {
        std::string name = d.readString(PLAYER_NAME_SIZE);

        PlayerInfo& player = *room.players[clientIdx];
        player.number = static_cast<uint8_t>(clientIdx + 1);
        player.hash = generateHash();
        std::strncpy(player.name, name.c_str(), PLAYER_NAME_SIZE - 1);

        Serializer s;
        s.writeU8(0x00);
        s.writeU64(player.hash);
        s.writeU8(player.number);
        sendToRoom(room, clientIdx, LobbyPacket::CONNECT_ACK, s.finalize());

        for (size_t i = 0; i < MAX_PLAYERS; ++i) {
            if (room.players[i] && i != clientIdx) {
                Serializer existingPlayerPayload;
                existingPlayerPayload.writeU8(room.players[i]->number);
                existingPlayerPayload.writeU64(room.players[i]->hash);
                existingPlayerPayload.writeString(room.players[i]->name, PLAYER_NAME_SIZE);
                sendToRoom(room, clientIdx, LobbyPacket::PLAYER_JOIN, existingPlayerPayload.finalize());

                if (room.players[i]->ready) {
                    Serializer readyPayload;
                    readyPayload.writeU8(room.players[i]->number);
                    sendToRoom(room, clientIdx, LobbyPacket::PLAYER_READY, readyPayload.finalize());
                }
            }
        }

        Serializer joinPayload;
        joinPayload.writeU8(player.number);
        joinPayload.writeU64(player.hash);
        joinPayload.writeString(player.name, PLAYER_NAME_SIZE);
        broadcastToRoom(room, LobbyPacket::PLAYER_JOIN, joinPayload.finalize());

        std::cout << "[RoomManager] Player " << name << " connected as #" << (int)player.number << std::endl;
    }

    void RoomManager::handleReady(Room& room, size_t clientIdx, Deserializer& d) {
        uint64_t hash = d.readU64();
        if (!room.players[clientIdx] || room.players[clientIdx]->hash != hash)
            return;

        room.players[clientIdx]->ready = !room.players[clientIdx]->ready;

        Serializer s;
        s.writeU8(room.players[clientIdx]->number);
        broadcastToRoom(room, LobbyPacket::PLAYER_READY, s.finalize());

        std::cout << "[RoomManager] Player #" << (int)room.players[clientIdx]->number << " is " << (room.players[clientIdx]->ready ? "ready" : "not ready") << std::endl;

        if (!room.players[clientIdx]->ready && room.countdownActive) {
            room.countdownActive = false;
            room.countdownTimer = 5.0f;
            Serializer countdownCancel;
            countdownCancel.writeU8(0);
            broadcastToRoom(room, LobbyPacket::COUNTDOWN, countdownCancel.finalize());
            std::cout << "[RoomManager] Countdown cancelled - player unready" << std::endl;
        }
    }

    void RoomManager::handleStart(Room& room, size_t) {
        if (!isRoomReady(room))
            return;

        room.inGame = true;

        Serializer s;
        s.writeU32(static_cast<uint32_t>(_rng()));
        s.writeU16(60);
        broadcastToRoom(room, LobbyPacket::GAME_START, s.finalize());

        std::cout << "[RoomManager] Game starting in room!" << std::endl;
    }

    void RoomManager::handleDisconnect(Room& room, size_t clientIdx) {
        removeClientFromRoom(room, clientIdx);
    }

    uint32_t RoomManager::createRoom(const std::string& name) {
        uint32_t roomId = _nextRoomId++;

        Room room;
        room.id = roomId;
        room.name = name;
        room.clients.resize(MAX_PLAYERS);
        room.players.resize(MAX_PLAYERS);
        room.lastUpdateTime = std::chrono::steady_clock::now();

        _rooms[roomId] = std::move(room);
        return roomId;
    }

    void RoomManager::removeClientFromRoom(Room& room, size_t idx) {
        if (idx >= MAX_PLAYERS || !room.players[idx])
            return;

        uint8_t playerNum = room.players[idx]->number;
        std::cout << "[RoomManager] Player #" << (int)playerNum << " disconnected from room" << std::endl;

        room.clients[idx].reset();
        room.players[idx].reset();

        if (playerNum > 0) {
            Serializer s;
            s.writeU8(playerNum);
            broadcastToRoom(room, LobbyPacket::PLAYER_LEFT, s.finalize());
        }
    }

    void RoomManager::updateRoomCountdown(Room& room) {
        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - room.lastUpdateTime).count();
        room.lastUpdateTime = now;

        if (isRoomReady(room) && !room.countdownActive && !room.inGame) {
            room.countdownActive = true;
            room.countdownTimer = 5.0f;
            room.lastBroadcastedSecond = -1;
            std::cout << "[RoomManager] All players ready in room! Starting countdown..." << std::endl;
        }

        if (room.countdownActive) {
            room.countdownTimer -= deltaTime;

            int currentSecond = static_cast<int>(std::ceil(room.countdownTimer));

            if (currentSecond != room.lastBroadcastedSecond && currentSecond > 0) {
                Serializer s;
                s.writeU8(static_cast<uint8_t>(currentSecond));
                broadcastToRoom(room, LobbyPacket::COUNTDOWN, s.finalize());
                std::cout << "[RoomManager] Countdown: " << currentSecond << "..." << std::endl;
                room.lastBroadcastedSecond = currentSecond;
            }

            if (room.countdownTimer <= 0.0f) {
                room.countdownActive = false;
                room.inGame = true;
                room.lastBroadcastedSecond = -1;

                Serializer s;
                s.writeU32(static_cast<uint32_t>(_rng()));
                s.writeU16(60);
                broadcastToRoom(room, LobbyPacket::GAME_START, s.finalize());

                std::cout << "[RoomManager] Game starting in room!" << std::endl;

                if (_onGameStart) {
                    _onGameStart(room.id, room);
                }
            }
        }
    }

    bool RoomManager::isRoomReady(const Room& room) const {
        size_t connectedPlayers = activePlayerCountInRoom(room);
        if (connectedPlayers < _minPlayersPerRoom)
            return false;
        for (const auto& p : room.players) {
            if (p && !p->ready)
                return false;
        }
        return true;
    }

    size_t RoomManager::findFreeSlotInRoom(const Room& room) const {
        for (size_t i = 0; i < MAX_PLAYERS; ++i) {
            if (!room.clients[i])
                return i;
        }
        return MAX_PLAYERS;
    }

    size_t RoomManager::activePlayerCountInRoom(const Room& room) const {
        size_t count = 0;
        for (const auto& p : room.players) {
            if (p)
                count++;
        }
        return count;
    }

    void RoomManager::sendTo(TcpSocket& client, LobbyPacket type, const std::vector<uint8_t>& payload) {
        std::cout << "[RoomManager] >> SEND " << lobbyPacketName(type) << " to client" << std::endl;
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        client.send(packet);
    }

    void RoomManager::sendToRoom(Room& room, size_t clientIdx, LobbyPacket type, const std::vector<uint8_t>& payload) {
        if (clientIdx >= MAX_PLAYERS || !room.clients[clientIdx])
            return;
        std::cout << "[RoomManager] >> SEND " << lobbyPacketName(type) << " to room client #" << (clientIdx + 1) << std::endl;
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        room.clients[clientIdx]->send(packet);
    }

    void RoomManager::broadcastToRoom(Room& room, LobbyPacket type, const std::vector<uint8_t>& payload) {
        std::cout << "[RoomManager] >> BROADCAST " << lobbyPacketName(type) << " to room" << std::endl;
        for (size_t i = 0; i < MAX_PLAYERS; ++i) {
            if (!room.clients[i])
                continue;
            Serializer s;
            s.writeU8(static_cast<uint8_t>(type));
            auto packet = s.finalize();
            packet.insert(packet.end(), payload.begin(), payload.end());
            room.clients[i]->send(packet);
        }
    }

    void RoomManager::broadcastRoomUpdate(uint32_t roomId) {
        auto it = _rooms.find(roomId);
        if (it == _rooms.end())
            return;

        RoomInfo info = getRoomInfo(it->second, roomId);

        Serializer s;
        s.writeU32(info.id);
        s.writeString(info.name, ROOM_NAME_SIZE);
        s.writeU8(info.playerCount);
        s.writeU8(info.maxPlayers);
        s.writeU8(info.inGame ? 1 : 0);

        for (const auto& client : _pendingClients) {
            if (client && client->isConnected()) {
                sendTo(*client, LobbyPacket::ROOM_UPDATE, s.finalize());
            }
        }
    }

    uint64_t RoomManager::generateHash() {
        return _rng();
    }

    RoomInfo RoomManager::getRoomInfo(const Room& room, uint32_t roomId) const {
        RoomInfo info;
        info.id = roomId;
        std::strncpy(info.name, room.name.c_str(), ROOM_NAME_SIZE - 1);
        info.playerCount = static_cast<uint8_t>(activePlayerCountInRoom(room));
        info.maxPlayers = MAX_PLAYERS;
        info.inGame = room.inGame;
        return info;
    }

    void RoomManager::printStatus() const {
        std::cout << "[RoomManager] Rooms: " << _rooms.size() << "/" << _maxRooms << " | Pending clients: " << _pendingClients.size() << std::endl;
        for (const auto& [roomId, room] : _rooms) {
            size_t playerCount = activePlayerCountInRoom(room);
            std::cout << "  - Room " << roomId << " '" << room.name << "': " << playerCount << "/" << MAX_PLAYERS << (room.inGame ? " [IN GAME]" : " [WAITING]") << std::endl;
        }
    }

}
