#pragma once

#include <Core/Module.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <memory>

namespace Network {

    using SocketId = std::uint32_t;
    constexpr SocketId INVALID_SOCKET_ID = 0;

    enum class SocketType {
        TCP,
        UDP
    };

    enum class SocketError {
        None,
        ConnectionRefused,
        Timeout,
        Disconnected,
        InvalidSocket,
        BufferOverflow,
        Unknown
    };

    struct Endpoint {
        std::string address;
        std::uint16_t port = 0;

        Endpoint() = default;
        Endpoint(const std::string& addr, std::uint16_t p) : address(addr), port(p) {}

        bool operator==(const Endpoint& other) const {
            return address == other.address && port == other.port;
        }

        bool operator!=(const Endpoint& other) const {
            return !(*this == other);
        }
    };

    struct SocketConfig {
        bool blocking = false;
        std::uint32_t sendBufferSize = 65536;
        std::uint32_t receiveBufferSize = 65536;
        bool reuseAddress = true;
    };

    struct ReceivedPacket {
        std::vector<std::uint8_t> data;
        Endpoint from;
    };

    struct SocketInfo {
        SocketId id;
        SocketType type;
        Endpoint localEndpoint;
        bool connected;
    };

    class INetworkModule : public RType::Core::IModule {
    public:
        ~INetworkModule() override = default;

        const char* GetName() const override = 0;
        RType::Core::ModulePriority GetPriority() const override = 0;
        bool Initialize(RType::Core::Engine* engine) override = 0;
        void Shutdown() override = 0;
        void Update(float deltaTime) override = 0;

        // TCP Socket Operations
        virtual SocketId CreateTcpSocket(const SocketConfig& config = SocketConfig{}) = 0;
        virtual bool ConnectTcp(SocketId socketId, const Endpoint& endpoint) = 0;
        virtual bool BindTcp(SocketId socketId, std::uint16_t port) = 0;
        virtual bool ListenTcp(SocketId socketId, std::uint32_t backlog = 10) = 0;
        virtual std::optional<SocketId> AcceptTcp(SocketId serverSocketId, Endpoint& clientEndpoint) = 0;
        virtual bool SendTcp(SocketId socketId, const std::vector<std::uint8_t>& data) = 0;
        virtual std::optional<std::vector<std::uint8_t>> ReceiveTcp(SocketId socketId, std::size_t maxSize = 2048) = 0;

        // UDP Socket Operations
        virtual SocketId CreateUdpSocket(const SocketConfig& config = SocketConfig{}) = 0;
        virtual bool BindUdp(SocketId socketId, std::uint16_t port) = 0;
        virtual bool SendUdp(SocketId socketId, const std::vector<std::uint8_t>& data, const Endpoint& to) = 0;
        virtual std::optional<ReceivedPacket> ReceiveUdp(SocketId socketId, std::size_t maxSize = 2048) = 0;

        // Common Socket Operations
        virtual void CloseSocket(SocketId socketId) = 0;
        virtual bool IsSocketValid(SocketId socketId) const = 0;
        virtual SocketError GetLastError() const = 0;
        virtual SocketInfo GetSocketInfo(SocketId socketId) const = 0;

        // Utility
        virtual Endpoint ResolveHostname(const std::string& hostname, std::uint16_t port) = 0;
        virtual std::string GetLocalAddress() const = 0;
    };

}

namespace std {
    template <>
    struct hash<Network::Endpoint> {
        std::size_t operator()(const Network::Endpoint& ep) const {
            std::size_t h1 = std::hash<std::string>{}(ep.address);
            std::size_t h2 = std::hash<std::uint16_t>{}(ep.port);
            return h1 ^ (h2 << 1);
        }
    };
}
