#pragma once

#include "network/UdpSocket.hpp"
#include "network/Protocol.hpp"
#include "network/Serializer.hpp"
#include "network/Deserializer.hpp"
#include <vector>
#include <random>
#include <optional>

namespace network {

    class LobbyServer {
    public:
        LobbyServer(uint16_t port, size_t maxPlayers = MAX_PLAYERS, size_t minPlayers = 2);

        void update();
        void printStatus() const;
        bool isGameReady() const;
        bool isGameStarted() const { return _gameStarted; }
        const std::vector<PlayerInfo>& getPlayers() const { return _players; }
        size_t playerCount() const { return _players.size(); }
    private:
        void handlePacket(const std::vector<uint8_t>& data, const Endpoint& sender);
        void handleConnect(Deserializer& d, const Endpoint& sender);
        void handleReady(Deserializer& d, const Endpoint& sender);
        void handleStart(const Endpoint& sender);
        void handleDisconnect(Deserializer& d, const Endpoint& sender);

        void sendTo(const Endpoint& dest, PacketType type, const std::vector<uint8_t>& payload);
        void broadcast(PacketType type, const std::vector<uint8_t>& payload);

        std::optional<size_t> findPlayerByHash(uint64_t hash);
        std::optional<size_t> findPlayerByEndpoint(const Endpoint& ep);
        uint64_t generateHash();

        UdpSocket _socket;
        std::vector<PlayerInfo> _players;
        std::vector<Endpoint> _endpoints;
        size_t _maxPlayers;
        size_t _minPlayers;
        bool _gameStarted = false;

        std::mt19937_64 _rng;
    };

}
