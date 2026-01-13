/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AsioNetworkModule implementation
*/

#include "AsioNetworkModule.hpp"
#include <array>
#include <system_error>

namespace Network {

    AsioNetworkModule::AsioNetworkModule() = default;

    AsioNetworkModule::~AsioNetworkModule() {
        Shutdown();
    }

    bool AsioNetworkModule::Initialize([[maybe_unused]] RType::Core::Engine* engine) {
        return true;
    }

    void AsioNetworkModule::Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tcpSockets.clear();
        m_udpSockets.clear();
        m_ioContext.stop();
    }

    void AsioNetworkModule::Update([[maybe_unused]] float deltaTime) {
        m_ioContext.poll();
    }

    SocketId AsioNetworkModule::CreateTcpSocket(const SocketConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        SocketId id = getNextSocketId();

        TcpSocketData data;
        data.socket = std::make_unique<asio::ip::tcp::socket>(m_ioContext);
        data.config = config;
        data.isServer = false;

        try {
            // Ensure native handle exists before applying options.
            data.socket->open(asio::ip::tcp::v4());
            if (config.reuseAddress) {
                data.socket->set_option(asio::socket_base::reuse_address(true));
            }
        } catch (...) {
            setLastError(SocketError::Unknown);
        }

        m_tcpSockets[id] = std::move(data);
        setLastError(SocketError::None);
        return id;
    }

    bool AsioNetworkModule::ConnectTcp(SocketId socketId, const Endpoint& endpoint) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tcpSockets.find(socketId);
        if (it == m_tcpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return false;
        }

        try {
            if (!it->second.socket->is_open()) {
                it->second.socket->open(asio::ip::tcp::v4());
                if (it->second.config.reuseAddress) {
                    it->second.socket->set_option(asio::socket_base::reuse_address(true));
                }
            }
            auto asioEndpoint = toAsioTcpEndpoint(endpoint);
            it->second.socket->connect(asioEndpoint);
            it->second.localEndpoint = fromAsioTcpEndpoint(it->second.socket->local_endpoint());
            setLastError(SocketError::None);
            return true;
        } catch (const std::system_error& e) {
            if (e.code() == asio::error::connection_refused) {
                setLastError(SocketError::ConnectionRefused);
            } else if (e.code() == asio::error::timed_out) {
                setLastError(SocketError::Timeout);
            } else {
                setLastError(SocketError::Unknown);
            }
            return false;
        }
    }

    bool AsioNetworkModule::BindTcp(SocketId socketId, std::uint16_t port) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tcpSockets.find(socketId);
        if (it == m_tcpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return false;
        }

        try {
            // Server sockets should bind on the acceptor (not on a separate socket),
            // otherwise ListenTcp() would attempt to bind the same port twice.
            asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);

            if (!it->second.acceptor) {
                it->second.acceptor = std::make_unique<asio::ip::tcp::acceptor>(m_ioContext);
                it->second.acceptor->open(endpoint.protocol());
                if (it->second.config.reuseAddress) {
                    it->second.acceptor->set_option(asio::socket_base::reuse_address(true));
                }
            }

            it->second.acceptor->bind(endpoint);
            it->second.localEndpoint = fromAsioTcpEndpoint(it->second.acceptor->local_endpoint());
            it->second.isServer = true;

            // Close/remove the unused socket handle for server mode.
            if (it->second.socket && it->second.socket->is_open()) {
                it->second.socket->close();
            }
            it->second.socket.reset();
            setLastError(SocketError::None);
            return true;
        } catch (...) {
            setLastError(SocketError::Unknown);
            return false;
        }
    }

    bool AsioNetworkModule::ListenTcp(SocketId socketId, std::uint32_t backlog) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tcpSockets.find(socketId);
        if (it == m_tcpSockets.end()) {
            setLastError(SocketError::InvalidSocket);
            return false;
        }

        try {
            if (!it->second.acceptor) {
                // Back-compat path: if BindTcp() bound a socket (older behavior), close it
                // BEFORE binding the acceptor to the same endpoint.
                if (!it->second.socket) {
                    setLastError(SocketError::InvalidSocket);
                    return false;
                }

                auto endpoint = it->second.socket->local_endpoint();
                if (it->second.socket->is_open()) {
                    it->second.socket->close();
                }
                it->second.socket.reset();

                it->second.acceptor = std::make_unique<asio::ip::tcp::acceptor>(m_ioContext);
                it->second.acceptor->open(endpoint.protocol());
                if (it->second.config.reuseAddress) {
                    it->second.acceptor->set_option(asio::socket_base::reuse_address(true));
                }
                it->second.acceptor->bind(endpoint);
                it->second.localEndpoint = fromAsioTcpEndpoint(it->second.acceptor->local_endpoint());
                it->second.isServer = true;
            }
            it->second.acceptor->listen(backlog);
            setLastError(SocketError::None);
            return true;
        } catch (...) {
            setLastError(SocketError::Unknown);
            return false;
        }
    }

    std::optional<SocketId> AsioNetworkModule::AcceptTcp(SocketId serverSocketId, Endpoint& clientEndpoint) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tcpSockets.find(serverSocketId);
        if (it == m_tcpSockets.end() || !it->second.acceptor) {
            setLastError(SocketError::InvalidSocket);
            return std::nullopt;
        }

        try {
            it->second.acceptor->non_blocking(true);
            SocketId clientId = getNextSocketId();

            TcpSocketData clientData;
            clientData.socket = std::make_unique<asio::ip::tcp::socket>(m_ioContext);
            clientData.config = it->second.config;
            clientData.isServer = false;

            it->second.acceptor->accept(*clientData.socket);

            clientEndpoint = fromAsioTcpEndpoint(clientData.socket->remote_endpoint());
            clientData.localEndpoint = fromAsioTcpEndpoint(clientData.socket->local_endpoint());

            m_tcpSockets[clientId] = std::move(clientData);
            setLastError(SocketError::None);
            return clientId;
        } catch (const std::system_error& e) {
            if (e.code() == asio::error::would_block) {
                setLastError(SocketError::None);
            } else {
                setLastError(SocketError::Unknown);
            }
            return std::nullopt;
        }
    }

    bool AsioNetworkModule::SendTcp(SocketId socketId, const std::vector<std::uint8_t>& data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tcpSockets.find(socketId);
        if (it == m_tcpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return false;
        }

        try {
            std::uint32_t size = static_cast<std::uint32_t>(data.size());
            std::vector<std::uint8_t> packet;
            packet.reserve(sizeof(size) + data.size());

            packet.push_back(static_cast<std::uint8_t>(size & 0xFF));
            packet.push_back(static_cast<std::uint8_t>((size >> 8) & 0xFF));
            packet.push_back(static_cast<std::uint8_t>((size >> 16) & 0xFF));
            packet.push_back(static_cast<std::uint8_t>((size >> 24) & 0xFF));
            packet.insert(packet.end(), data.begin(), data.end());

            asio::write(*it->second.socket, asio::buffer(packet));
            setLastError(SocketError::None);
            return true;
        } catch (const std::system_error& e) {
            if (e.code() == asio::error::eof || e.code() == asio::error::connection_reset) {
                setLastError(SocketError::Disconnected);
            } else {
                setLastError(SocketError::Unknown);
            }
            return false;
        }
    }

    std::optional<std::vector<std::uint8_t>> AsioNetworkModule::ReceiveTcp(SocketId socketId, std::size_t maxSize) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_tcpSockets.find(socketId);
        if (it == m_tcpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return std::nullopt;
        }

        it->second.socket->non_blocking(true);

        std::array<std::uint8_t, 4096> temp{};
        asio::error_code ec;
        std::size_t bytesRead = it->second.socket->read_some(asio::buffer(temp), ec);

        if (!ec && bytesRead > 0) {
            auto& buf = it->second.recvBuffer;
            buf.insert(buf.end(), temp.begin(), temp.begin() + static_cast<std::ptrdiff_t>(bytesRead));
        } else if (ec == asio::error::would_block) {
            // No data available right now.
        } else if (ec == asio::error::eof || ec == asio::error::connection_reset) {
            setLastError(SocketError::Disconnected);
            return std::nullopt;
        } else if (ec) {
            setLastError(SocketError::Unknown);
            return std::nullopt;
        }

        auto& buffer = it->second.recvBuffer;
        if (buffer.size() < 4) {
            setLastError(SocketError::None);
            return std::nullopt;
        }

        std::uint32_t size = static_cast<std::uint32_t>(buffer[0]) |
                            (static_cast<std::uint32_t>(buffer[1]) << 8) |
                            (static_cast<std::uint32_t>(buffer[2]) << 16) |
                            (static_cast<std::uint32_t>(buffer[3]) << 24);

        if (size > maxSize) {
            // Protocol violation / too-large message: drop buffered data to avoid getting stuck.
            buffer.clear();
            setLastError(SocketError::BufferOverflow);
            return std::nullopt;
        }

        if (buffer.size() < 4 + static_cast<std::size_t>(size)) {
            setLastError(SocketError::None);
            return std::nullopt;
        }

        std::vector<std::uint8_t> data;
        data.reserve(size);
        data.insert(data.end(), buffer.begin() + 4, buffer.begin() + 4 + static_cast<std::ptrdiff_t>(size));

        buffer.erase(buffer.begin(), buffer.begin() + 4 + static_cast<std::ptrdiff_t>(size));
        setLastError(SocketError::None);
        return data;
    }

    SocketId AsioNetworkModule::CreateUdpSocket(const SocketConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        SocketId id = getNextSocketId();

        UdpSocketData data;
        data.socket = std::make_unique<asio::ip::udp::socket>(m_ioContext, asio::ip::udp::v4());
        data.config = config;

        if (config.reuseAddress) {
            data.socket->set_option(asio::socket_base::reuse_address(true));
        }

        m_udpSockets[id] = std::move(data);
        setLastError(SocketError::None);
        return id;
    }

    bool AsioNetworkModule::BindUdp(SocketId socketId, std::uint16_t port) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_udpSockets.find(socketId);
        if (it == m_udpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return false;
        }

        try {
            asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), port);
            it->second.socket->bind(endpoint);
            it->second.localEndpoint = fromAsioUdpEndpoint(it->second.socket->local_endpoint());
            setLastError(SocketError::None);
            return true;
        } catch (...) {
            setLastError(SocketError::Unknown);
            return false;
        }
    }

    bool AsioNetworkModule::SendUdp(SocketId socketId, const std::vector<std::uint8_t>& data, const Endpoint& to) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_udpSockets.find(socketId);
        if (it == m_udpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return false;
        }

        try {
            auto endpoint = toAsioUdpEndpoint(to);
            it->second.socket->send_to(asio::buffer(data), endpoint);
            setLastError(SocketError::None);
            return true;
        } catch (...) {
            setLastError(SocketError::Unknown);
            return false;
        }
    }

    std::optional<ReceivedPacket> AsioNetworkModule::ReceiveUdp(SocketId socketId, std::size_t maxSize) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_udpSockets.find(socketId);
        if (it == m_udpSockets.end() || !it->second.socket) {
            setLastError(SocketError::InvalidSocket);
            return std::nullopt;
        }

        try {
            it->second.socket->non_blocking(true);

            ReceivedPacket packet;
            packet.data.resize(maxSize);
            asio::ip::udp::endpoint senderEndpoint;

            std::size_t bytesReceived = it->second.socket->receive_from(
                asio::buffer(packet.data), senderEndpoint);

            packet.data.resize(bytesReceived);
            packet.from = fromAsioUdpEndpoint(senderEndpoint);

            setLastError(SocketError::None);
            return packet;
        } catch (const std::system_error& e) {
            if (e.code() == asio::error::would_block) {
                setLastError(SocketError::None);
            } else {
                setLastError(SocketError::Unknown);
            }
            return std::nullopt;
        }
    }

    void AsioNetworkModule::CloseSocket(SocketId socketId) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto tcpIt = m_tcpSockets.find(socketId);
        if (tcpIt != m_tcpSockets.end()) {
            if (tcpIt->second.socket && tcpIt->second.socket->is_open()) {
                tcpIt->second.socket->close();
            }
            if (tcpIt->second.acceptor && tcpIt->second.acceptor->is_open()) {
                tcpIt->second.acceptor->close();
            }
            m_tcpSockets.erase(tcpIt);
            setLastError(SocketError::None);
            return;
        }

        auto udpIt = m_udpSockets.find(socketId);
        if (udpIt != m_udpSockets.end()) {
            if (udpIt->second.socket && udpIt->second.socket->is_open()) {
                udpIt->second.socket->close();
            }
            m_udpSockets.erase(udpIt);
            setLastError(SocketError::None);
            return;
        }

        setLastError(SocketError::InvalidSocket);
    }

    bool AsioNetworkModule::IsSocketValid(SocketId socketId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_tcpSockets.find(socketId) != m_tcpSockets.end() ||
               m_udpSockets.find(socketId) != m_udpSockets.end();
    }

    SocketError AsioNetworkModule::GetLastError() const {
        return m_lastError;
    }

    SocketInfo AsioNetworkModule::GetSocketInfo(SocketId socketId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        SocketInfo info{};
        info.id = socketId;

        auto tcpIt = m_tcpSockets.find(socketId);
        if (tcpIt != m_tcpSockets.end()) {
            info.type = SocketType::TCP;
            info.localEndpoint = tcpIt->second.localEndpoint;
            info.connected = tcpIt->second.socket && tcpIt->second.socket->is_open();
            return info;
        }

        auto udpIt = m_udpSockets.find(socketId);
        if (udpIt != m_udpSockets.end()) {
            info.type = SocketType::UDP;
            info.localEndpoint = udpIt->second.localEndpoint;
            info.connected = udpIt->second.socket && udpIt->second.socket->is_open();
            return info;
        }

        return info;
    }

    Endpoint AsioNetworkModule::ResolveHostname(const std::string& hostname, std::uint16_t port) {
        try {
            asio::ip::tcp::resolver resolver(m_ioContext);
            auto results = resolver.resolve(hostname, std::to_string(port));

            if (results.begin() != results.end()) {
                return fromAsioTcpEndpoint(*results.begin());
            }
        } catch (...) {
            setLastError(SocketError::Unknown);
        }
        return Endpoint{hostname, port};
    }

    std::string AsioNetworkModule::GetLocalAddress() const {
        try {
            asio::ip::tcp::resolver resolver(const_cast<asio::io_context&>(m_ioContext));
            asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
            auto results = resolver.resolve(query);

            for (const auto& entry : results) {
                if (entry.endpoint().address().is_v4()) {
                    return entry.endpoint().address().to_string();
                }
            }
        } catch (...) {}
        return "127.0.0.1";
    }

    SocketId AsioNetworkModule::getNextSocketId() {
        return m_nextSocketId++;
    }

    void AsioNetworkModule::setLastError(SocketError error) {
        m_lastError = error;
    }

    asio::ip::tcp::endpoint AsioNetworkModule::toAsioTcpEndpoint(const Endpoint& ep) {
        asio::ip::address addr = asio::ip::make_address(ep.address);
        return asio::ip::tcp::endpoint(addr, ep.port);
    }

    asio::ip::udp::endpoint AsioNetworkModule::toAsioUdpEndpoint(const Endpoint& ep) {
        asio::ip::address addr = asio::ip::make_address(ep.address);
        return asio::ip::udp::endpoint(addr, ep.port);
    }

    Endpoint AsioNetworkModule::fromAsioTcpEndpoint(const asio::ip::tcp::endpoint& ep) {
        return Endpoint{ep.address().to_string(), ep.port()};
    }

    Endpoint AsioNetworkModule::fromAsioUdpEndpoint(const asio::ip::udp::endpoint& ep) {
        return Endpoint{ep.address().to_string(), ep.port()};
    }

}
