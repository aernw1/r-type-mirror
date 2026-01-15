/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomClient
*/

#include "RoomClient.hpp"
#include <cstring>
#include <iostream>

namespace network {

    static const char* lobbyPacketName(LobbyPacket type) {
        switch (type) {
        case LobbyPacket::LIST_ROOMS_REQ: return "LIST_ROOMS_REQ";
        case LobbyPacket::LIST_ROOMS_ACK: return "LIST_ROOMS_ACK";
        case LobbyPacket::CREATE_ROOM_REQ: return "CREATE_ROOM_REQ";
        case LobbyPacket::CREATE_ROOM_ACK: return "CREATE_ROOM_ACK";
        case LobbyPacket::JOIN_ROOM_REQ: return "JOIN_ROOM_REQ";
        case LobbyPacket::JOIN_ROOM_ACK: return "JOIN_ROOM_ACK";
        case LobbyPacket::ROOM_UPDATE: return "ROOM_UPDATE";
        default: return "UNKNOWN";
        }
    }

    RoomClient::RoomClient(Network::INetworkModule* network, const std::string& serverAddr, uint16_t port)
        : _socket(network, serverAddr, port) {
        if (_socket.isConnected()) {
            std::cout << "[RoomClient] Connected to server" << std::endl;
        } else {
            std::cout << "[RoomClient] Failed to connect to server" << std::endl;
            if (_onConnectionError) {
                _onConnectionError("Failed to connect to server");
            }
        }
    }

    void RoomClient::requestRooms() {
        if (!_socket.isConnected())
            return;
        send(LobbyPacket::LIST_ROOMS_REQ);
    }

    void RoomClient::createRoom(const std::string& roomName) {
        if (!_socket.isConnected())
            return;
        Serializer s;
        s.writeString(roomName, ROOM_NAME_SIZE);
        send(LobbyPacket::CREATE_ROOM_REQ, s.finalize());
    }

    void RoomClient::joinRoom(uint32_t roomId) {
        if (!_socket.isConnected())
            return;
        Serializer s;
        s.writeU32(roomId);
        send(LobbyPacket::JOIN_ROOM_REQ, s.finalize());
    }

    void RoomClient::update() {
        if (!_socket.isConnected())
            return;
        while (auto data = _socket.receive()) {
            handlePacket(*data);
        }
    }

    void RoomClient::handlePacket(const std::vector<uint8_t>& data) {
        if (data.empty())
            return;

        Deserializer d(data);
        auto type = static_cast<LobbyPacket>(d.readU8());

        std::cout << "[RoomClient] << RECV " << lobbyPacketName(type) << std::endl;

        switch (type) {
        case LobbyPacket::LIST_ROOMS_ACK:
            handleListRoomsAck(d);
            break;
        case LobbyPacket::CREATE_ROOM_ACK:
            handleCreateRoomAck(d);
            break;
        case LobbyPacket::JOIN_ROOM_ACK:
            handleJoinRoomAck(d);
            break;
        case LobbyPacket::ROOM_UPDATE:
            handleRoomUpdate(d);
            break;
        default:
            std::cout << "[RoomClient] Unknown packet type: " << lobbyPacketName(type) << std::endl;
            break;
        }
    }

    void RoomClient::handleListRoomsAck(Deserializer& d) {
        uint8_t roomCount = d.readU8();
        _rooms.clear();

        for (uint8_t i = 0; i < roomCount; ++i) {
            RoomInfo room;
            room.id = d.readU32();
            std::string name = d.readString(ROOM_NAME_SIZE);
            std::strncpy(room.name, name.c_str(), ROOM_NAME_SIZE - 1);
            room.playerCount = d.readU8();
            room.maxPlayers = d.readU8();
            room.inGame = d.readU8() != 0;
            _rooms.push_back(room);
        }

        std::cout << "[RoomClient] Received " << (int)roomCount << " rooms" << std::endl;

        if (_onRoomList) {
            _onRoomList(_rooms);
        }
    }

    void RoomClient::handleCreateRoomAck(Deserializer& d) {
        uint8_t status = d.readU8();
        uint32_t roomId = d.readU32();

        if (status == 0x00) {
            std::cout << "[RoomClient] Room created with ID " << roomId << std::endl;
            if (_onRoomCreated) {
                _onRoomCreated(roomId);
            }
        } else {
            std::cout << "[RoomClient] Failed to create room (status=" << (int)status << ")" << std::endl;
            if (_onConnectionError) {
                _onConnectionError("Failed to create room: max rooms reached");
            }
        }
    }

    void RoomClient::handleJoinRoomAck(Deserializer& d) {
        auto status = static_cast<JoinRoomStatus>(d.readU8());

        if (status == JoinRoomStatus::SUCCESS) {
            _joinedRoom = true;
            std::cout << "[RoomClient] Successfully joined room" << std::endl;
        } else {
            std::string errorMsg;
            switch (status) {
            case JoinRoomStatus::ROOM_FULL:
                errorMsg = "Room is full";
                break;
            case JoinRoomStatus::ROOM_NOT_FOUND:
                errorMsg = "Room not found";
                break;
            case JoinRoomStatus::ROOM_IN_GAME:
                errorMsg = "Game already in progress";
                break;
            default:
                errorMsg = "Unknown error";
                break;
            }
            std::cout << "[RoomClient] Failed to join room: " << errorMsg << std::endl;
        }

        if (_onRoomJoined) {
            _onRoomJoined(status);
        }
    }

    void RoomClient::handleRoomUpdate(Deserializer& d) {
        RoomInfo room;
        room.id = d.readU32();
        std::string name = d.readString(ROOM_NAME_SIZE);
        std::strncpy(room.name, name.c_str(), ROOM_NAME_SIZE - 1);
        room.playerCount = d.readU8();
        room.maxPlayers = d.readU8();
        room.inGame = d.readU8() != 0;

        bool found = false;
        for (auto& r : _rooms) {
            if (r.id == room.id) {
                r = room;
                found = true;
                break;
            }
        }
        if (!found) {
            _rooms.push_back(room);
        }

        std::cout << "[RoomClient] Room " << room.id << " updated: " << room.name << " (" << (int)room.playerCount << "/" << (int)room.maxPlayers << ")" << (room.inGame ? " [IN GAME]" : "") << std::endl;

        if (_onRoomUpdate) {
            _onRoomUpdate(room);
        }
    }

    void RoomClient::send(LobbyPacket type, const std::vector<uint8_t>& payload) {
        std::cout << "[RoomClient] >> SEND " << lobbyPacketName(type) << std::endl;
        Serializer s;
        s.writeU8(static_cast<uint8_t>(type));
        auto packet = s.finalize();
        packet.insert(packet.end(), payload.begin(), payload.end());
        _socket.send(packet);
    }

}
