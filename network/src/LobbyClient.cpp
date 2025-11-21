#include "network/LobbyClient.hpp"
#include <cstring>
#include <iostream>

namespace network {

    LobbyClient::LobbyClient(const std::string& serverAddr, uint16_t port)
        : _socket(serverAddr, port) {}

    void LobbyClient::connect(const std::string& playerName) {
        Serializer s;
        s.writeString(playerName, PLAYER_NAME_SIZE);
        send(PacketType::CONNECT_REQ, s.finalize());
    }

    void LobbyClient::ready() {
        if (!_connected)
            return;
        Serializer s;
        s.writeU64(_myInfo.hash);
        send(PacketType::READY_REQ, s.finalize());
    }

    void LobbyClient::requestStart() {
        if (!_connected)
            return;
        send(PacketType::START_REQ);
    }

    void LobbyClient::disconnect() {
        if (!_connected)
            return;
        Serializer s;
        s.writeU64(_myInfo.hash);
        send(PacketType::DISCONNECT, s.finalize());
        _connected = false;
    }

    void LobbyClient::update() {
        auto packets = _socket.receive();
        for (auto& [data, sender] : packets) {
            handlePacket(data);
        }
    }

    void LobbyClient::handlePacket(const std::vector<uint8_t>& data) {
        if (data.empty())
            return;

        Deserializer d(data);
        auto type = static_cast<PacketType>(d.readU8());

        switch (type) {
        case PacketType::CONNECT_ACK:
            handleConnectAck(d);
            break;
        case PacketType::PLAYER_JOIN:
            handlePlayerJoin(d);
            break;
        case PacketType::PLAYER_READY:
            handlePlayerReady(d);
            break;
        case PacketType::GAME_START:
            handleGameStart(d);
            break;
        default:
            break;
        }
    }

    void LobbyClient::handleConnectAck(Deserializer& d) {
        uint8_t status = d.readU8();
        if (status != 0) {
            std::cout << "[Client] Connection refused (status=" << (int)status << ")" << std::endl;
            return;
        }

        _myInfo.hash = d.readU64();
        _myInfo.number = d.readU8();
        _connected = true;

        std::cout << "[Client] Connected as player #" << (int)_myInfo.number << std::endl;
    }

    void LobbyClient::handlePlayerJoin(Deserializer& d) {
        PlayerInfo p;
        p.number = d.readU8();
        std::string name = d.readString(PLAYER_NAME_SIZE);
        std::strncpy(p.name, name.c_str(), PLAYER_NAME_SIZE - 1);

        _players.push_back(p);
        std::cout << "[Client] Player " << name << " joined as #" << (int)p.number << std::endl;
    }

    void LobbyClient::handlePlayerReady(Deserializer& d) {
        uint8_t num = d.readU8();
        for (auto& p : _players) {
            if (p.number == num) {
                p.ready = true;
                break;
            }
        }
        std::cout << "[Client] Player #" << (int)num << " is ready" << std::endl;
    }

    void LobbyClient::handleGameStart(Deserializer& d) {
        _gameSeed = d.readU32();
        uint16_t tickRate = d.readU16();
        _gameStarted = true;

        std::cout << "[Client] Game started! Seed=" << _gameSeed << " TickRate=" << tickRate
                  << std::endl;
    }

    void LobbyClient::send(PacketType type, const std::vector<uint8_t>& payload) {
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        _socket.send(packet);
    }

}
