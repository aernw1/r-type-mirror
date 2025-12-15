/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TcpServer
*/

#include "TcpServer.hpp"

namespace network {

    TcpServer::TcpServer(uint16_t port) : _acceptor(_io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
        _acceptor.non_blocking(true);
    }

    std::optional<TcpSocket> TcpServer::accept() {
        asio::error_code ec;
        asio::ip::tcp::socket socket(_io);
        _acceptor.accept(socket, ec);
        if (ec)
            return std::nullopt;
        return TcpSocket(std::move(socket));
    }

}
