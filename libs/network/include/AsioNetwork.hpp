#pragma once

#include "Network/INetwork.hpp"
#include "UdpSocket.hpp"
#include "Endpoint.hpp"
#include <memory>
#include <unordered_map>
#include <mutex>

namespace network {

    class AsioNetwork : public Network::INetwork {
    public:
        AsioNetwork();
        ~AsioNetwork() override;

        const char* GetName() const override;
        RType::Core::ModulePriority GetPriority() const override;
        bool Initialize(RType::Core::Engine* engine) override;
        void Shutdown() override;
        void Update(float deltaTime) override;

        bool StartServer(const Network::NetworkConfig& config) override;
        void StopServer() override;

        bool Connect(const std::string& address, std::uint16_t port) override;
        void Disconnect() override;

        void SendReliable(Network::ClientId clientId,
                          const std::vector<std::uint8_t>& data) override;
        void SendUnreliable(Network::ClientId clientId,
                            const std::vector<std::uint8_t>& data) override;
        void Broadcast(const std::vector<std::uint8_t>& data) override;
        void BroadcastReliable(const std::vector<std::uint8_t>& data) override;

        std::queue<Network::Packet> ReceivePackets() override;

        std::vector<Network::ClientConnection> GetConnectedClients() const override;
        bool IsServer() const override;
        bool IsConnected() const override;
        bool IsRunning() const override;

        Network::NetworkStats GetStats() const override;
    private:
        Network::ClientId FindOrCreateClient(const Endpoint& endpoint);
        void RemoveClient(Network::ClientId clientId);

        std::unique_ptr<UdpSocket> _socket;
        std::unordered_map<Network::ClientId, Endpoint> _clients;
        std::unordered_map<std::string, Network::ClientId> _endpointToClient;
        Endpoint _serverEndpoint;
        Network::ClientId _nextClientId = 1;
        Network::ClientId _myClientId = Network::INVALID_CLIENT_ID;
        std::size_t _maxClients = 0;
        bool _isServer = false;
        bool _isConnected = false;
        bool _initialized = false;
        Network::NetworkStats _stats;
        mutable std::mutex _mutex;
    };

}
