#pragma once

#include <asio.hpp>
#include <vector>
#include <cstdint>
#include "network/Endpoint.hpp"

namespace network {

    class UdpSocket {
    public:
        static constexpr size_t MAX_PACKET_SIZE = 2048;

        explicit UdpSocket(uint16_t port);
        UdpSocket(const std::string& address, uint16_t port);
        ~UdpSocket() = default;

        UdpSocket(const UdpSocket&) = delete;
        UdpSocket& operator=(const UdpSocket&) = delete;
        UdpSocket(UdpSocket&&) = default;
        UdpSocket& operator=(UdpSocket&&) = default;

        void sendTo(const std::vector<uint8_t>& data, const Endpoint& dest);
        void send(const std::vector<uint8_t>& data);
        std::vector<std::pair<std::vector<uint8_t>, Endpoint>> receive();

        uint16_t localPort() const;
        bool isClient() const { return _isClient; }
    private:
        asio::io_context _io;
        asio::ip::udp::socket _socket;
        Endpoint _remoteEndpoint;
        bool _isClient = false;
    };

}
