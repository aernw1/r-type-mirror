/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyServer
*/

#include "LobbyServer.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace network {

    LobbyServer::LobbyServer(uint16_t port, size_t maxPlayers, size_t minPlayers) : _server(port), _maxPlayers(maxPlayers), _minPlayers(minPlayers), _rng(std::random_device{}()) {
        _clients.resize(_maxPlayers);
        _players.resize(_maxPlayers);
        std::cout << "[Lobby] Server started on port " << port << " (min=" << _minPlayers << ", max=" << _maxPlayers << ")" << std::endl;
    }

    void LobbyServer::update() {
        acceptNewClients();
        processClients();
    }

    void LobbyServer::acceptNewClients() {
        while (auto newClient = _server.accept()) {
            size_t slot = findFreeSlot();
            if (slot >= _maxPlayers) {
                Serializer s;
                s.writeU8(0x01);
                std::vector<uint8_t> packet = {static_cast<uint8_t>(LobbyPacket::CONNECT_ACK)};
                auto payload = s.finalize();
                packet.insert(packet.end(), payload.begin(), payload.end());
                newClient->send(packet);
                continue;
            }
            std::cout << "[Lobby] New connection accepted (slot #" << (slot + 1) << ")" << std::endl;
            _clients[slot] = std::move(*newClient);
            _players[slot] = PlayerInfo{};
        }
    }

    void LobbyServer::processClients() {
        for (size_t idx = 0; idx < _maxPlayers; ++idx) {
            if (!_clients[idx])
                continue;
            if (!_clients[idx]->isConnected()) {
                removeClient(idx);
                continue;
            }
            while (auto data = _clients[idx]->receive()) {
                handlePacket(idx, *data);
            }
        }
    }

    static const char* lobbyPacketName(LobbyPacket type) {
        switch (type) {
        case LobbyPacket::CONNECT_REQ: return "CONNECT_REQ";
        case LobbyPacket::CONNECT_ACK: return "CONNECT_ACK";
        case LobbyPacket::PLAYER_JOIN: return "PLAYER_JOIN";
        case LobbyPacket::READY_REQ: return "READY_REQ";
        case LobbyPacket::PLAYER_READY: return "PLAYER_READY";
        case LobbyPacket::START_REQ: return "START_REQ";
        case LobbyPacket::GAME_START: return "GAME_START";
        case LobbyPacket::DISCONNECT: return "DISCONNECT";
        case LobbyPacket::PLAYER_LEFT: return "PLAYER_LEFT";
        case LobbyPacket::ERROR: return "ERROR";
        default: return "UNKNOWN";
        }
    }

    void LobbyServer::handlePacket(size_t clientIdx, const std::vector<uint8_t>& data) {
        if (data.empty())
            return;

        Deserializer d(data);
        auto type = static_cast<LobbyPacket>(d.readU8());

        std::cout << "[Server] << RECV " << lobbyPacketName(type) << " from client #" << (clientIdx + 1) << std::endl;

        switch (type) {
        case LobbyPacket::CONNECT_REQ:
            handleConnect(clientIdx, d);
            break;
        case LobbyPacket::READY_REQ:
            handleReady(clientIdx, d);
            break;
        case LobbyPacket::START_REQ:
            handleStart(clientIdx);
            break;
        case LobbyPacket::DISCONNECT:
            handleDisconnect(clientIdx);
            break;
        default:
            break;
        }
    }

    void LobbyServer::handleConnect(size_t clientIdx, Deserializer& d) {
        std::string name = d.readString(PLAYER_NAME_SIZE);

        PlayerInfo& player = *_players[clientIdx];
        player.number = static_cast<uint8_t>(clientIdx + 1);
        player.hash = generateHash();
        std::strncpy(player.name, name.c_str(), PLAYER_NAME_SIZE - 1);

        Serializer s;
        s.writeU8(0x00);
        s.writeU64(player.hash);
        s.writeU8(player.number);
        sendTo(clientIdx, LobbyPacket::CONNECT_ACK, s.finalize());

        Serializer joinPayload;
        joinPayload.writeU8(player.number);
        joinPayload.writeString(player.name, PLAYER_NAME_SIZE);
        broadcast(LobbyPacket::PLAYER_JOIN, joinPayload.finalize());

        std::cout << "[Lobby] Player " << name << " joined as #" << (int)player.number << std::endl;
    }

    void LobbyServer::handleReady(size_t clientIdx, Deserializer& d) {
        uint64_t hash = d.readU64();
        if (!_players[clientIdx] || _players[clientIdx]->hash != hash)
            return;

        _players[clientIdx]->ready = true;

        Serializer s;
        s.writeU8(_players[clientIdx]->number);
        broadcast(LobbyPacket::PLAYER_READY, s.finalize());

        std::cout << "[Lobby] Player #" << (int)_players[clientIdx]->number << " is ready" << std::endl;
    }

    void LobbyServer::handleStart(size_t /*clientIdx*/) {
        if (!isGameReady())
            return;

        _gameStarted = true;

        Serializer s;
        s.writeU32(static_cast<uint32_t>(_rng()));
        s.writeU16(60);
        broadcast(LobbyPacket::GAME_START, s.finalize());

        std::cout << "[Lobby] Game starting!" << std::endl;
    }

    void LobbyServer::handleDisconnect(size_t clientIdx) {
        removeClient(clientIdx);
    }

    void LobbyServer::removeClient(size_t idx) {
        if (idx >= _maxPlayers || !_players[idx])
            return;

        uint8_t playerNum = _players[idx]->number;
        std::cout << "[Lobby] Player #" << (int)playerNum << " disconnected" << std::endl;

        _clients[idx].reset();
        _players[idx].reset();

        if (playerNum > 0) {
            Serializer s;
            s.writeU8(playerNum);
            broadcast(LobbyPacket::PLAYER_LEFT, s.finalize());
        }
    }

    bool LobbyServer::isGameReady() const {
        size_t connectedPlayers = activePlayerCount();
        if (connectedPlayers < _minPlayers)
            return false;
        for (const auto& p : _players) {
            if (p && !p->ready)
                return false;
        }
        return true;
    }

    void LobbyServer::printStatus() const {
        size_t readyCount = 0;
        size_t connectedCount = activePlayerCount();
        for (const auto& p : _players) {
            if (p && p->ready)
                readyCount++;
        }
        std::cout << "[Lobby] Players: " << connectedCount << "/" << _maxPlayers << " | Ready: " << readyCount << "/" << connectedCount << " | Min to start: " << _minPlayers << std::endl;
    }

    void LobbyServer::sendTo(size_t clientIdx, LobbyPacket type, const std::vector<uint8_t>& payload) {
        if (clientIdx >= _maxPlayers || !_clients[clientIdx])
            return;
        std::cout << "[Server] >> SEND " << lobbyPacketName(type) << " to client #" << (clientIdx + 1) << std::endl;
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        _clients[clientIdx]->send(packet);
    }

    void LobbyServer::broadcast(LobbyPacket type, const std::vector<uint8_t>& payload) {
        std::cout << "[Server] >> BROADCAST " << lobbyPacketName(type) << " to all clients" << std::endl;
        for (size_t i = 0; i < _maxPlayers; ++i) {
            if (!_clients[i])
                continue;
            Serializer s;
            s.writeU8(static_cast<uint8_t>(type));
            auto packet = s.finalize();
            packet.insert(packet.end(), payload.begin(), payload.end());
            _clients[i]->send(packet);
        }
    }

    std::optional<size_t> LobbyServer::findPlayerByHash(uint64_t hash) {
        for (size_t i = 0; i < _maxPlayers; ++i) {
            if (_players[i] && _players[i]->hash == hash)
                return i;
        }
        return std::nullopt;
    }

    size_t LobbyServer::findFreeSlot() const {
        for (size_t i = 0; i < _maxPlayers; ++i) {
            if (!_clients[i])
                return i;
        }
        return _maxPlayers;
    }

    size_t LobbyServer::activePlayerCount() const {
        size_t count = 0;
        for (const auto& p : _players) {
            if (p)
                count++;
        }
        return count;
    }

    uint64_t LobbyServer::generateHash() { return _rng(); }

}
