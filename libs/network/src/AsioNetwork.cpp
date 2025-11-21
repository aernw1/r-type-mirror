#include "AsioNetwork.hpp"

namespace network {

    AsioNetwork::AsioNetwork() = default;

    AsioNetwork::~AsioNetwork() { Shutdown(); }

    const char* AsioNetwork::GetName() const { return "AsioNetwork"; }

    RType::Core::ModulePriority AsioNetwork::GetPriority() const {
        return RType::Core::ModulePriority::High;
    }

    bool AsioNetwork::Initialize(RType::Core::Engine* /*engine*/) {
        if (_initialized)
            return true;
        _initialized = true;
        return true;
    }

    void AsioNetwork::Shutdown() {
        if (!_initialized)
            return;
        Disconnect();
        StopServer();
        _initialized = false;
    }

    void AsioNetwork::Update(float /*deltaTime*/) {}

    bool AsioNetwork::StartServer(const Network::NetworkConfig& config) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isConnected || _isServer)
            return false;
        _socket = std::make_unique<UdpSocket>(config.port);
        _maxClients = config.maxClients;
        _isServer = true;
        return true;
    }

    void AsioNetwork::StopServer() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_isServer)
            return;
        _socket.reset();
        _clients.clear();
        _endpointToClient.clear();
        _nextClientId = 1;
        _isServer = false;
    }

    bool AsioNetwork::Connect(const std::string& address, std::uint16_t port) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_isServer || _isConnected)
            return false;
        _socket = std::make_unique<UdpSocket>(address, port);
        _serverEndpoint = Endpoint(address, port);
        _isConnected = true;
        return true;
    }

    void AsioNetwork::Disconnect() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_isConnected)
            return;
        _socket.reset();
        _isConnected = false;
        _myClientId = Network::INVALID_CLIENT_ID;
    }

    void AsioNetwork::SendReliable(Network::ClientId clientId,
                                   const std::vector<std::uint8_t>& data) {
        SendUnreliable(clientId, data);
    }

    void AsioNetwork::SendUnreliable(Network::ClientId clientId,
                                     const std::vector<std::uint8_t>& data) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_socket)
            return;
        if (_isServer) {
            auto it = _clients.find(clientId);
            if (it != _clients.end()) {
                _socket->sendTo(data, it->second);
                _stats.bytesSent += data.size();
                _stats.packetsSent++;
            }
        } else {
            _socket->send(data);
            _stats.bytesSent += data.size();
            _stats.packetsSent++;
        }
    }

    void AsioNetwork::Broadcast(const std::vector<std::uint8_t>& data) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_socket || !_isServer)
            return;
        for (const auto& [id, endpoint] : _clients) {
            _socket->sendTo(data, endpoint);
            _stats.bytesSent += data.size();
            _stats.packetsSent++;
        }
    }

    void AsioNetwork::BroadcastReliable(const std::vector<std::uint8_t>& data) { Broadcast(data); }

    std::queue<Network::Packet> AsioNetwork::ReceivePackets() {
        std::queue<Network::Packet> packets;
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_socket)
            return packets;

        auto rawPackets = _socket->receive();
        for (auto& [data, sender] : rawPackets) {
            Network::Packet packet;
            packet.data = std::move(data);
            _stats.bytesReceived += packet.data.size();
            _stats.packetsReceived++;

            if (_isServer) {
                packet.clientId = FindOrCreateClient(sender);
            } else {
                packet.clientId = Network::SERVER_ID;
            }
            packets.push(std::move(packet));
        }
        return packets;
    }

    Network::ClientId AsioNetwork::FindOrCreateClient(const Endpoint& endpoint) {
        std::string key = endpoint.address() + ":" + std::to_string(endpoint.port());
        auto it = _endpointToClient.find(key);
        if (it != _endpointToClient.end())
            return it->second;

        if (_clients.size() >= _maxClients)
            return Network::INVALID_CLIENT_ID;

        Network::ClientId id = _nextClientId++;
        _clients[id] = endpoint;
        _endpointToClient[key] = id;
        return id;
    }

    void AsioNetwork::RemoveClient(Network::ClientId clientId) {
        auto it = _clients.find(clientId);
        if (it == _clients.end())
            return;

        std::string key = it->second.address() + ":" + std::to_string(it->second.port());
        _endpointToClient.erase(key);
        _clients.erase(it);
    }

    std::vector<Network::ClientConnection> AsioNetwork::GetConnectedClients() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<Network::ClientConnection> connections;
        for (const auto& [id, ep] : _clients) {
            Network::ClientConnection conn;
            conn.id = id;
            conn.address = ep.address();
            conn.port = ep.port();
            conn.connected = true;
            connections.push_back(conn);
        }
        return connections;
    }

    bool AsioNetwork::IsServer() const { return _isServer; }

    bool AsioNetwork::IsConnected() const { return _isConnected; }

    bool AsioNetwork::IsRunning() const { return _isServer || _isConnected; }

    Network::NetworkStats AsioNetwork::GetStats() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _stats;
    }

}
