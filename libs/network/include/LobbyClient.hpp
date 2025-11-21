/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyClient
*/

#pragma once

#include "TcpSocket.hpp"
#include "Protocol.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include <string>
#include <vector>
#include <functional>

namespace network {

    class LobbyClient {
    public:
        LobbyClient(const std::string& serverAddr, uint16_t port);

        void connect(const std::string& playerName);
        void ready();
        void requestStart();
        void disconnect();
        void update();

        bool isConnected() const { return _socket.isConnected() && _joined; }
        bool isGameStarted() const { return _gameStarted; }
        const PlayerInfo& getMyInfo() const { return _myInfo; }
        const std::vector<PlayerInfo>& getPlayers() const { return _players; }
        uint32_t getGameSeed() const { return _gameSeed; }

        void onPlayerLeft(std::function<void(uint8_t)> callback) { _onPlayerLeft = callback; }

    private:
        void handlePacket(const std::vector<uint8_t>& data);
        void handleConnectAck(Deserializer& d);
        void handlePlayerJoin(Deserializer& d);
        void handlePlayerReady(Deserializer& d);
        void handlePlayerLeft(Deserializer& d);
        void handleGameStart(Deserializer& d);

        void send(LobbyPacket type, const std::vector<uint8_t>& payload = {});

        TcpSocket _socket;
        PlayerInfo _myInfo;
        std::vector<PlayerInfo> _players;

        bool _joined = false;
        bool _gameStarted = false;
        uint32_t _gameSeed = 0;

        std::function<void(uint8_t)> _onPlayerLeft;
    };

}
