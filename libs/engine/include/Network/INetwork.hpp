#pragma once

#include "Core/Module.hpp"
#include <cstdint>
#include <queue>
#include <string>
#include <vector>

namespace Network {

    using ClientId = std::uint32_t;

    constexpr ClientId INVALID_CLIENT_ID = 0;
    constexpr ClientId SERVER_ID = 0;

    struct Packet {
        ClientId clientId = INVALID_CLIENT_ID;
        std::vector<std::uint8_t> data;
    };

    struct ClientConnection {
        ClientId id = INVALID_CLIENT_ID;
        std::string address;
        std::uint16_t port = 0;
        bool connected = false;
    };

    struct NetworkConfig {
        std::uint16_t port = 4242;
        std::size_t maxClients = 4;
        std::uint32_t tickRate = 60;
    };

    struct NetworkStats {
        std::uint64_t bytesSent = 0;
        std::uint64_t bytesReceived = 0;
        std::uint32_t packetsSent = 0;
        std::uint32_t packetsReceived = 0;
        float latency = 0.0f;
    };

    class INetwork : public RType::Core::IModule {
    public:
        ~INetwork() override = default;

        const char* GetName() const override = 0;
        RType::Core::ModulePriority GetPriority() const override = 0;
        bool Initialize(RType::Core::Engine* engine) override = 0;
        void Shutdown() override = 0;
        void Update(float deltaTime) override = 0;

        virtual bool StartServer(const NetworkConfig& config) = 0;
        virtual void StopServer() = 0;

        virtual bool Connect(const std::string& address, std::uint16_t port) = 0;
        virtual void Disconnect() = 0;

        virtual void SendReliable(ClientId clientId, const std::vector<std::uint8_t>& data) = 0;
        virtual void SendUnreliable(ClientId clientId, const std::vector<std::uint8_t>& data) = 0;
        virtual void Broadcast(const std::vector<std::uint8_t>& data) = 0;
        virtual void BroadcastReliable(const std::vector<std::uint8_t>& data) = 0;

        virtual std::queue<Packet> ReceivePackets() = 0;

        virtual std::vector<ClientConnection> GetConnectedClients() const = 0;
        virtual bool IsServer() const = 0;
        virtual bool IsConnected() const = 0;
        virtual bool IsRunning() const = 0;

        virtual NetworkStats GetStats() const = 0;
    };

}
