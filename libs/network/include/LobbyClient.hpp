/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyClient
*/

#pragma once

#include "NetworkTcpSocket.hpp"
#include "Protocol.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include <string>
#include <vector>
#include <functional>

namespace network {

    class LobbyClient {
    public:
        LobbyClient(Network::INetworkModule* network, const std::string& serverAddr, uint16_t port);
        explicit LobbyClient(NetworkTcpSocket&& socket);

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
        uint8_t getCountdownSeconds() const { return _countdownSeconds; }

        void onPlayerLeft(std::function<void(uint8_t)> callback) { _onPlayerLeft = callback; }
        void onConnectionError(std::function<void(const std::string&)> callback) { _onConnectionError = callback; }
        void onCountdown(std::function<void(uint8_t)> callback) { _onCountdown = callback; }
    private:
        void handlePacket(const std::vector<uint8_t>& data);
        void handleConnectAck(Deserializer& d);
        void handlePlayerJoin(Deserializer& d);
        void handlePlayerReady(Deserializer& d);
        void handlePlayerLeft(Deserializer& d);
        void handleCountdown(Deserializer& d);
        void handleGameStart(Deserializer& d);

        void send(LobbyPacket type, const std::vector<uint8_t>& payload = {});

        NetworkTcpSocket _socket;
        PlayerInfo _myInfo;
        std::vector<PlayerInfo> _players;

        bool _joined = false;
        bool _gameStarted = false;
        uint32_t _gameSeed = 0;
        uint8_t _countdownSeconds = 0;

        std::function<void(uint8_t)> _onPlayerLeft;
        std::function<void(const std::string&)> _onConnectionError;
        std::function<void(uint8_t)> _onCountdown;
    };

}
