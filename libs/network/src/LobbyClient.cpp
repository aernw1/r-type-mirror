/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LobbyClient
*/

#include "LobbyClient.hpp"
#include <cstring>
#include <iostream>
#include <algorithm>

namespace network {

    LobbyClient::LobbyClient(const std::string& serverAddr, uint16_t port) : _socket(serverAddr, port) {
        if (_socket.isConnected())
            std::cout << "[Client] Connected to server" << std::endl;
        else
            std::cout << "[Client] Failed to connect to server" << std::endl;
    }

    void LobbyClient::connect(const std::string& playerName) {
        if (!_socket.isConnected())
            return;
        Serializer s;
        s.writeString(playerName, PLAYER_NAME_SIZE);
        send(LobbyPacket::CONNECT_REQ, s.finalize());
    }

    void LobbyClient::ready() {
        if (!isConnected())
            return;
        Serializer s;
        s.writeU64(_myInfo.hash);
        send(LobbyPacket::READY_REQ, s.finalize());
    }

    void LobbyClient::requestStart() {
        if (!isConnected())
            return;
        send(LobbyPacket::START_REQ);
    }

    void LobbyClient::disconnect() {
        if (!isConnected())
            return;
        Serializer s;
        s.writeU64(_myInfo.hash);
        send(LobbyPacket::DISCONNECT, s.finalize());
        _socket.disconnect();
        _joined = false;
    }

    void LobbyClient::update() {
        if (!_socket.isConnected())
            return;
        while (auto data = _socket.receive()) {
            handlePacket(*data);
        }
    }

    static const char* lobbyPacketName(LobbyPacket type) {
        switch (type) {
        case LobbyPacket::CONNECT_REQ:
            return "CONNECT_REQ";
        case LobbyPacket::CONNECT_ACK:
            return "CONNECT_ACK";
        case LobbyPacket::PLAYER_JOIN:
            return "PLAYER_JOIN";
        case LobbyPacket::READY_REQ:
            return "READY_REQ";
        case LobbyPacket::PLAYER_READY:
            return "PLAYER_READY";
        case LobbyPacket::START_REQ:
            return "START_REQ";
        case LobbyPacket::GAME_START:
            return "GAME_START";
        case LobbyPacket::DISCONNECT:
            return "DISCONNECT";
        case LobbyPacket::PLAYER_LEFT:
            return "PLAYER_LEFT";
        case LobbyPacket::COUNTDOWN:
            return "COUNTDOWN";
        case LobbyPacket::ERROR_MSG:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    void LobbyClient::handlePacket(const std::vector<uint8_t>& data) {
        if (data.empty())
            return;

        Deserializer d(data);
        auto type = static_cast<LobbyPacket>(d.readU8());

        std::cout << "[Client] << RECV " << lobbyPacketName(type) << std::endl;

        switch (type) {
        case LobbyPacket::CONNECT_ACK:
            handleConnectAck(d);
            break;
        case LobbyPacket::PLAYER_JOIN:
            handlePlayerJoin(d);
            break;
        case LobbyPacket::PLAYER_READY:
            handlePlayerReady(d);
            break;
        case LobbyPacket::PLAYER_LEFT:
            handlePlayerLeft(d);
            break;
        case LobbyPacket::COUNTDOWN:
            handleCountdown(d);
            break;
        case LobbyPacket::GAME_START:
            handleGameStart(d);
            break;
        default:
            break;
        }
    }

    void LobbyClient::handleConnectAck(Deserializer& d) {
        uint8_t status = d.readU8();
        if (status != 0) {
            std::string errorMsg = "Server is full";
            if (status == 0x01) {
                errorMsg = "Server is full";
            } else {
                errorMsg = "Connection refused (status=" + std::to_string((int)status) + ")";
            }
            std::cout << "[Client] " << errorMsg << std::endl;
            if (_onConnectionError) {
                _onConnectionError(errorMsg);
            }
            _socket.disconnect();
            return;
        }

        _myInfo.hash = d.readU64();
        _myInfo.number = d.readU8();
        _joined = true;

        std::cout << "[Client] Joined as player #" << (int)_myInfo.number << std::endl;
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
                p.ready = !p.ready;
                std::cout << "[Client] Player #" << (int)num << " is " << (p.ready ? "ready" : "not ready") << std::endl;
                break;
            }
        }
    }

    void LobbyClient::handlePlayerLeft(Deserializer& d) {
        uint8_t num = d.readU8();
        _players.erase(std::remove_if(_players.begin(), _players.end(), [num](const PlayerInfo& p) { return p.number == num; }), _players.end());
        std::cout << "[Client] Player #" << (int)num << " left" << std::endl;
        if (_onPlayerLeft)
            _onPlayerLeft(num);
    }

    void LobbyClient::handleCountdown(Deserializer& d) {
        uint8_t seconds = d.readU8();
        _countdownSeconds = seconds;
        if (seconds == 0) {
            std::cout << "[Client] Countdown cancelled" << std::endl;
        } else {
            std::cout << "[Client] Game starting in " << (int)seconds << " seconds..." << std::endl;
        }
        if (_onCountdown) {
            _onCountdown(seconds);
        }
    }

    void LobbyClient::handleGameStart(Deserializer& d) {
        _gameSeed = d.readU32();
        uint16_t tickRate = d.readU16();
        _gameStarted = true;

        std::cout << "[Client] Game started! Seed=" << _gameSeed << " TickRate=" << tickRate << std::endl;
    }

    void LobbyClient::send(LobbyPacket type, const std::vector<uint8_t>& payload) {
        std::cout << "[Client] >> SEND " << lobbyPacketName(type) << std::endl;
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        _socket.send(packet);
    }

}
