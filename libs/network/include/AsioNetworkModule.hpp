#pragma once

#include "INetworkModule.hpp"
#include <asio.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace Network {

    class AsioNetworkModule : public INetworkModule {
    public:
        AsioNetworkModule();
        ~AsioNetworkModule() override;

        const char* GetName() const override { return "AsioNetworkModule"; }
        RType::Core::ModulePriority GetPriority() const override {
            return RType::Core::ModulePriority::High;
        }
        bool Initialize(RType::Core::Engine* engine) override;
        void Shutdown() override;
        void Update(float deltaTime) override;

        // TCP Socket Operations
        SocketId CreateTcpSocket(const SocketConfig& config = SocketConfig{}) override;
        bool ConnectTcp(SocketId socketId, const Endpoint& endpoint) override;
        bool BindTcp(SocketId socketId, std::uint16_t port) override;
        bool ListenTcp(SocketId socketId, std::uint32_t backlog = 10) override;
        std::optional<SocketId> AcceptTcp(SocketId serverSocketId, Endpoint& clientEndpoint) override;
        bool SendTcp(SocketId socketId, const std::vector<std::uint8_t>& data) override;
        std::optional<std::vector<std::uint8_t>> ReceiveTcp(SocketId socketId, std::size_t maxSize = 2048) override;

        // UDP Socket Operations
        SocketId CreateUdpSocket(const SocketConfig& config = SocketConfig{}) override;
        bool BindUdp(SocketId socketId, std::uint16_t port) override;
        bool SendUdp(SocketId socketId, const std::vector<std::uint8_t>& data, const Endpoint& to) override;
        std::optional<ReceivedPacket> ReceiveUdp(SocketId socketId, std::size_t maxSize = 2048) override;

        // Common Socket Operations
        void CloseSocket(SocketId socketId) override;
        bool IsSocketValid(SocketId socketId) const override;
        SocketError GetLastError() const override;
        SocketInfo GetSocketInfo(SocketId socketId) const override;

        // Utility
        Endpoint ResolveHostname(const std::string& hostname, std::uint16_t port) override;
        std::string GetLocalAddress() const override;

    private:
        struct TcpSocketData {
            std::unique_ptr<asio::ip::tcp::socket> socket;
            std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
            SocketConfig config;
            Endpoint localEndpoint;
            bool isServer = false;
        };

        struct UdpSocketData {
            std::unique_ptr<asio::ip::udp::socket> socket;
            SocketConfig config;
            Endpoint localEndpoint;
        };

        SocketId getNextSocketId();
        void setLastError(SocketError error);
        asio::ip::tcp::endpoint toAsioTcpEndpoint(const Endpoint& ep);
        asio::ip::udp::endpoint toAsioUdpEndpoint(const Endpoint& ep);
        Endpoint fromAsioTcpEndpoint(const asio::ip::tcp::endpoint& ep);
        Endpoint fromAsioUdpEndpoint(const asio::ip::udp::endpoint& ep);

        asio::io_context m_ioContext;
        std::unordered_map<SocketId, TcpSocketData> m_tcpSockets;
        std::unordered_map<SocketId, UdpSocketData> m_udpSockets;
        SocketId m_nextSocketId = 1;
        SocketError m_lastError = SocketError::None;
        mutable std::mutex m_mutex;
    };

}

extern "C" {
    #if defined(_WIN32) || defined(_WIN64)
        #define RTYPE_MODULE_EXPORT __declspec(dllexport)
    #else
        #define RTYPE_MODULE_EXPORT __attribute__((visibility("default")))
    #endif

    RTYPE_MODULE_EXPORT RType::Core::IModule* CreateModule() {
        return new Network::AsioNetworkModule();
    }

    RTYPE_MODULE_EXPORT void DestroyModule(RType::Core::IModule* module) {
        delete module;
    }
}
