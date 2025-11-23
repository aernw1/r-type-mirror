/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyServer
*/

#pragma once

#include "TcpServer.hpp"
#include "TcpSocket.hpp"
#include "Protocol.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include <vector>
#include <random>
#include <optional>
#include <memory>

namespace network {

    class LobbyServer {
    public:
        LobbyServer(uint16_t port, size_t maxPlayers = MAX_PLAYERS, size_t minPlayers = 2);

        void update();
        void printStatus() const;
        bool isGameReady() const;
        bool isGameStarted() const { return _gameStarted; }
        const std::vector<std::optional<PlayerInfo>>& getPlayers() const { return _players; }
        size_t playerCount() const { return activePlayerCount(); }
    private:
        void acceptNewClients();
        void processClients();
        void handlePacket(size_t clientIdx, const std::vector<uint8_t>& data);
        void handleConnect(size_t clientIdx, Deserializer& d);
        void handleReady(size_t clientIdx, Deserializer& d);
        void handleStart(size_t clientIdx);
        void handleDisconnect(size_t clientIdx);

        void removeClient(size_t idx);
        void sendTo(size_t clientIdx, LobbyPacket type, const std::vector<uint8_t>& payload = {});
        void broadcast(LobbyPacket type, const std::vector<uint8_t>& payload = {});

        std::optional<size_t> findPlayerByHash(uint64_t hash);
        uint64_t generateHash();

        size_t findFreeSlot() const;
        size_t activePlayerCount() const;

        TcpServer _server;
        std::vector<std::optional<TcpSocket>> _clients;
        std::vector<std::optional<PlayerInfo>> _players;
        size_t _maxPlayers;
        size_t _minPlayers;
        bool _gameStarted = false;

        std::mt19937_64 _rng;
    };

}
