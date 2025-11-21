#include "LobbyServer.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace network {

    LobbyServer::LobbyServer(uint16_t port, size_t maxPlayers, size_t minPlayers)
        : _socket(port), _maxPlayers(maxPlayers), _minPlayers(minPlayers),
          _rng(std::random_device{}()) {
        std::cout << "[Lobby] Server started (min=" << _minPlayers << ", max=" << _maxPlayers << ")"
                  << std::endl;
    }

    void LobbyServer::update() {
        auto packets = _socket.receive();
        for (auto& [data, sender] : packets) {
            handlePacket(data, sender);
        }
    }

    void LobbyServer::handlePacket(const std::vector<uint8_t>& data, const Endpoint& sender) {
        if (data.empty())
            return;

        Deserializer d(data);
        auto type = static_cast<PacketType>(d.readU8());

        switch (type) {
        case PacketType::CONNECT_REQ:
            handleConnect(d, sender);
            break;
        case PacketType::READY_REQ:
            handleReady(d, sender);
            break;
        case PacketType::START_REQ:
            handleStart(sender);
            break;
        case PacketType::DISCONNECT:
            handleDisconnect(d, sender);
            break;
        default:
            break;
        }
    }

    void LobbyServer::handleConnect(Deserializer& d, const Endpoint& sender) {
        if (_players.size() >= _maxPlayers || _gameStarted) {
            Serializer s;
            s.writeU8(0x01);
            sendTo(sender, PacketType::CONNECT_ACK, s.finalize());
            return;
        }

        std::string name = d.readString(PLAYER_NAME_SIZE);

        PlayerInfo player;
        player.number = static_cast<uint8_t>(_players.size() + 1);
        player.hash = generateHash();
        std::strncpy(player.name, name.c_str(), PLAYER_NAME_SIZE - 1);

        _players.push_back(player);
        _endpoints.push_back(sender);

        Serializer s;
        s.writeU8(0x00);
        s.writeU64(player.hash);
        s.writeU8(player.number);
        sendTo(sender, PacketType::CONNECT_ACK, s.finalize());

        Serializer joinPayload;
        joinPayload.writeU8(player.number);
        joinPayload.writeString(player.name, PLAYER_NAME_SIZE);
        broadcast(PacketType::PLAYER_JOIN, joinPayload.finalize());

        std::cout << "[Lobby] Player " << name << " joined as #" << (int)player.number << std::endl;
    }

    void LobbyServer::handleReady(Deserializer& d, const Endpoint& /*sender*/) {
        uint64_t hash = d.readU64();
        auto idx = findPlayerByHash(hash);
        if (!idx)
            return;

        _players[*idx].ready = true;

        Serializer s;
        s.writeU8(_players[*idx].number);
        broadcast(PacketType::PLAYER_READY, s.finalize());

        std::cout << "[Lobby] Player #" << (int)_players[*idx].number << " is ready" << std::endl;
    }

    void LobbyServer::handleStart(const Endpoint& /*sender*/) {
        if (!isGameReady())
            return;

        _gameStarted = true;

        Serializer s;
        s.writeU32(static_cast<uint32_t>(_rng()));
        s.writeU16(60);
        broadcast(PacketType::GAME_START, s.finalize());

        std::cout << "[Lobby] Game starting!" << std::endl;
    }

    void LobbyServer::handleDisconnect(Deserializer& d, const Endpoint& /*sender*/) {
        uint64_t hash = d.readU64();
        auto idx = findPlayerByHash(hash);
        if (!idx)
            return;

        std::cout << "[Lobby] Player #" << (int)_players[*idx].number << " disconnected"
                  << std::endl;

        _players.erase(_players.begin() + static_cast<std::ptrdiff_t>(*idx));
        _endpoints.erase(_endpoints.begin() + static_cast<std::ptrdiff_t>(*idx));
    }

    bool LobbyServer::isGameReady() const {
        if (_players.size() < _minPlayers)
            return false;
        return std::all_of(_players.begin(), _players.end(),
                           [](const PlayerInfo& p) { return p.ready; });
    }

    void LobbyServer::printStatus() const {
        size_t readyCount = 0;
        for (const auto& p : _players) {
            if (p.ready)
                readyCount++;
        }
        std::cout << "[Lobby] Players: " << _players.size() << "/" << _maxPlayers
                  << " | Ready: " << readyCount << "/" << _players.size()
                  << " | Min to start: " << _minPlayers << std::endl;
    }

    void LobbyServer::sendTo(const Endpoint& dest, PacketType type,
                             const std::vector<uint8_t>& payload) {
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        _socket.sendTo(packet, dest);
    }

    void LobbyServer::broadcast(PacketType type, const std::vector<uint8_t>& payload) {
        for (const auto& ep : _endpoints) {
            sendTo(ep, type, payload);
        }
    }

    std::optional<size_t> LobbyServer::findPlayerByHash(uint64_t hash) {
        for (size_t i = 0; i < _players.size(); ++i) {
            if (_players[i].hash == hash)
                return i;
        }
        return std::nullopt;
    }

    std::optional<size_t> LobbyServer::findPlayerByEndpoint(const Endpoint& ep) {
        for (size_t i = 0; i < _endpoints.size(); ++i) {
            if (_endpoints[i] == ep)
                return i;
        }
        return std::nullopt;
    }

    uint64_t LobbyServer::generateHash() { return _rng(); }

}
