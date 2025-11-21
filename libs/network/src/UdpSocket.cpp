#include "UdpSocket.hpp"
#include <stdexcept>

namespace network {

    UdpSocket::UdpSocket(uint16_t port) : _socket(_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
        _socket.non_blocking(true);
    }

    UdpSocket::UdpSocket(const std::string& address, uint16_t port) : _socket(_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)), _remoteEndpoint(address, port), _isClient(true) {
        _socket.non_blocking(true);
    }

    void UdpSocket::sendTo(const std::vector<uint8_t>& data, const Endpoint& dest) {
        asio::error_code ec;
        _socket.send_to(asio::buffer(data), dest.raw(), 0, ec);
    }

    void UdpSocket::send(const std::vector<uint8_t>& data) { sendTo(data, _remoteEndpoint); }

    std::vector<std::pair<std::vector<uint8_t>, Endpoint>> UdpSocket::receive() {
        std::vector<std::pair<std::vector<uint8_t>, Endpoint>> packets;
        std::array<uint8_t, MAX_PACKET_SIZE> buffer{};

        while (true) {
            asio::ip::udp::endpoint sender;
            asio::error_code ec;

            size_t bytes = _socket.receive_from(asio::buffer(buffer), sender, 0, ec);

            if (ec == asio::error::would_block || bytes == 0)
                break;
            if (ec)
                continue;

            packets.emplace_back(std::vector<uint8_t>(buffer.begin(), buffer.begin() + bytes), Endpoint(sender));
        }

        return packets;
    }

    uint16_t UdpSocket::localPort() const { return _socket.local_endpoint().port(); }

}
