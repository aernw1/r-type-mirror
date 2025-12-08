/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TcpSocket
*/

#pragma once

#include <asio.hpp>
#include <vector>
#include <cstdint>
#include <string>
#include <optional>
#include <memory>

namespace network {

    class TcpSocket {
    public:
        static constexpr size_t MAX_PACKET_SIZE = 65536; // j'ai tenté d'augmenter la taille du buffer mais ça ne fonctionne pas dsl

        TcpSocket(const std::string& address, uint16_t port);
        explicit TcpSocket(asio::ip::tcp::socket socket);
        ~TcpSocket() = default;

        TcpSocket(const TcpSocket&) = delete;
        TcpSocket& operator=(const TcpSocket&) = delete;
        TcpSocket(TcpSocket&& other) noexcept;
        TcpSocket& operator=(TcpSocket&& other) noexcept;

        void send(const std::vector<uint8_t>& data);
        std::optional<std::vector<uint8_t>> receive();

        bool isConnected() const { return _connected; }
        void disconnect();
    private:
        std::unique_ptr<asio::io_context> _io;
        std::unique_ptr<asio::ip::tcp::socket> _socket;
        bool _connected = false;
    };

}
