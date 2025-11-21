#pragma once

#include "UdpSocket.hpp"
#include "Protocol.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include <string>
#include <vector>

namespace network {

    class LobbyClient {
    public:
        LobbyClient(const std::string& serverAddr, uint16_t port);

        void connect(const std::string& playerName);
        void ready();
        void requestStart();
        void disconnect();
        void update();

        bool isConnected() const { return _connected; }
        bool isGameStarted() const { return _gameStarted; }
        const PlayerInfo& getMyInfo() const { return _myInfo; }
        const std::vector<PlayerInfo>& getPlayers() const { return _players; }
        uint32_t getGameSeed() const { return _gameSeed; }
    private:
        void handlePacket(const std::vector<uint8_t>& data);
        void handleConnectAck(Deserializer& d);
        void handlePlayerJoin(Deserializer& d);
        void handlePlayerReady(Deserializer& d);
        void handleGameStart(Deserializer& d);

        void send(PacketType type, const std::vector<uint8_t>& payload = {});

        UdpSocket _socket;
        PlayerInfo _myInfo;
        std::vector<PlayerInfo> _players;

        bool _connected = false;
        bool _gameStarted = false;
        uint32_t _gameSeed = 0;
    };

}
