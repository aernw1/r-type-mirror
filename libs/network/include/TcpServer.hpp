/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TcpServer
*/

#pragma once

#include "TcpSocket.hpp"
#include <memory>
#include <optional>

namespace network {

    class TcpServer {
    public:
        explicit TcpServer(uint16_t port);
        ~TcpServer() = default;

        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        std::optional<TcpSocket> accept();
        uint16_t port() const { return _acceptor.local_endpoint().port(); }

    private:
        asio::io_context _io;
        asio::ip::tcp::acceptor _acceptor;
    };

}
