/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomClient - Client for room selection
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

    class RoomClient {
    public:
        RoomClient(Network::INetworkModule* network, const std::string& serverAddr, uint16_t port);

        void requestRooms();
        void createRoom(const std::string& roomName);
        void joinRoom(uint32_t roomId);
        void update();

        bool isConnected() const { return _socket.isConnected(); }
        bool hasJoinedRoom() const { return _joinedRoom; }
        uint32_t getJoinedRoomId() const { return _joinedRoomId; }
        const std::vector<RoomInfo>& getRooms() const { return _rooms; }

        NetworkTcpSocket releaseSocket() { return std::move(_socket); }

        void onRoomList(std::function<void(const std::vector<RoomInfo>&)> callback) { _onRoomList = callback; }
        void onRoomCreated(std::function<void(uint32_t roomId)> callback) { _onRoomCreated = callback; }
        void onRoomJoined(std::function<void(JoinRoomStatus status)> callback) { _onRoomJoined = callback; }
        void onRoomUpdate(std::function<void(const RoomInfo&)> callback) { _onRoomUpdate = callback; }
        void onConnectionError(std::function<void(const std::string&)> callback) { _onConnectionError = callback; }

    private:
        void handlePacket(const std::vector<uint8_t>& data);
        void handleListRoomsAck(Deserializer& d);
        void handleCreateRoomAck(Deserializer& d);
        void handleJoinRoomAck(Deserializer& d);
        void handleRoomUpdate(Deserializer& d);

        void send(LobbyPacket type, const std::vector<uint8_t>& payload = {});

        NetworkTcpSocket _socket;
        std::vector<RoomInfo> _rooms;
        bool _joinedRoom = false;
        uint32_t _joinedRoomId = 0;

        std::function<void(const std::vector<RoomInfo>&)> _onRoomList;
        std::function<void(uint32_t)> _onRoomCreated;
        std::function<void(JoinRoomStatus)> _onRoomJoined;
        std::function<void(const RoomInfo&)> _onRoomUpdate;
        std::function<void(const std::string&)> _onConnectionError;
    };

}
