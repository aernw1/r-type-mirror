/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TcpSocket
*/

#include "TcpSocket.hpp"

namespace network {

    TcpSocket::TcpSocket(const std::string& address, uint16_t port) : _io(std::make_unique<asio::io_context>()), _socket(std::make_unique<asio::ip::tcp::socket>(*_io)) {
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(address), port);
        asio::error_code ec;
        _socket->connect(endpoint, ec);
        _connected = !ec;
        if (_connected)
            _socket->non_blocking(true);
    }

    TcpSocket::TcpSocket(asio::ip::tcp::socket socket) : _io(nullptr), _socket(std::make_unique<asio::ip::tcp::socket>(std::move(socket))), _connected(true) {
        _socket->non_blocking(true);
    }

    TcpSocket::TcpSocket(TcpSocket&& other) noexcept : _io(std::move(other._io)), _socket(std::move(other._socket)), _connected(other._connected) {
        other._connected = false;
    }

    TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept {
        if (this != &other) {
            _io = std::move(other._io);
            _socket = std::move(other._socket);
            _connected = other._connected;
            other._connected = false;
        }
        return *this;
    }

    void TcpSocket::send(const std::vector<uint8_t>& data) {
        if (!_connected || !_socket)
            return;
        uint32_t size = static_cast<uint32_t>(data.size());
        std::vector<uint8_t> packet(4 + data.size());
        packet[0] = size & 0xFF;
        packet[1] = (size >> 8) & 0xFF;
        packet[2] = (size >> 16) & 0xFF;
        packet[3] = (size >> 24) & 0xFF;
        std::copy(data.begin(), data.end(), packet.begin() + 4);

        asio::error_code ec;
        asio::write(*_socket, asio::buffer(packet), ec);
        if (ec)
            _connected = false;
    }

    std::optional<std::vector<uint8_t>> TcpSocket::receive() {
        if (!_connected || !_socket)
            return std::nullopt;

        asio::error_code ec;
        std::array<uint8_t, 4> sizeBuffer{};

        size_t available = _socket->available(ec);
        if (ec || available < 4)
            return std::nullopt;

        asio::read(*_socket, asio::buffer(sizeBuffer), ec);
        if (ec) {
            if (ec != asio::error::would_block)
                _connected = false;
            return std::nullopt;
        }

        uint32_t size = sizeBuffer[0] | (sizeBuffer[1] << 8) | (sizeBuffer[2] << 16) | (sizeBuffer[3] << 24);
        if (size > MAX_PACKET_SIZE) {
            _connected = false;
            return std::nullopt;
        }

        std::vector<uint8_t> data(size);
        asio::read(*_socket, asio::buffer(data), ec);
        if (ec) {
            _connected = false;
            return std::nullopt;
        }

        return data;
    }

    void TcpSocket::disconnect() {
        if (!_connected || !_socket)
            return;
        asio::error_code ec;
        _socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        _socket->close(ec);
        _connected = false;
    }

}
