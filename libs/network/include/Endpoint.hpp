/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Endpoint
*/

#pragma once

#include <asio.hpp>
#include <string>
#include <cstdint>

namespace network {

    class Endpoint {
    public:
        Endpoint() = default;
        Endpoint(const asio::ip::udp::endpoint& ep)
            : _endpoint(ep) {}
        Endpoint(const std::string& address, uint16_t port)
            : _endpoint(asio::ip::make_address(address), port) {}

        std::string address() const { return _endpoint.address().to_string(); }
        uint16_t port() const { return _endpoint.port(); }
        const asio::ip::udp::endpoint& raw() const { return _endpoint; }
        asio::ip::udp::endpoint& raw() { return _endpoint; }

        bool operator==(const Endpoint& other) const { return _endpoint == other._endpoint; }
    private:
        asio::ip::udp::endpoint _endpoint;
    };

}
