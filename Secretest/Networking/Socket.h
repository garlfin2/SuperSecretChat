//
// Created by scion on 1/11/2026.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <print>
#include <list>
#include <mutex>
#include <vector>

struct addrinfo;
using SOCKET = uint64_t;
using WSAEVENT = void*;

#define LOCALHOST 0x0100007F

#define SOCKET_BUF_SIZE 512

namespace Secretest
{
    class SocketContext
    {
    public:
        explicit SocketContext(uint16_t maxConnections = 32);
        ~SocketContext();
    };

    struct Address
    {
        Address(uint32_t ip, uint16_t port) : IP(ip), Port(port) {};
        Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port) : IPBytes{ a, b, c, d }, Port(port) {};
        Address() = default;

        explicit operator std::string()
        {
            return std::format("{}.{}.{}.{}:{}", IPBytes[3], IPBytes[2], IPBytes[1], IPBytes[0], Port);
        }

        union
        {
            uint32_t IP;
            uint8_t IPBytes[4];
        };

        uint16_t Port;


        bool operator==(const Address& b) const { return Port == b.Port && IP == b.IP; }
    };

    enum class IConnectionStatusQueryType
    {
        Read = 1 << 0,
        Write = 1 << 1,
        Exception = 1 << 2
    };

    constexpr uint64_t InvalidSocket = ~0;

    class ConnectionCreationException : public std::exception
    {
    public:
        const char* what() const noexcept override { return "Failed to create socket."; }
    };

    class IConnection
    {
    public:
        explicit IConnection(Address address);

        IConnection() = default;
        IConnection(const IConnection&) = delete;
        IConnection& operator=(const IConnection&) = delete;
        IConnection(IConnection&& b) noexcept;
        IConnection& operator=(IConnection&& b) noexcept;

        [[nodiscard]] Address GetAddress() const { return Address_; }
        [[nodiscard]] int32_t GetStatus(IConnectionStatusQueryType type) const;
        [[nodiscard]] bool IsOpen() const { return Socket_ != InvalidSocket; }

        virtual void Close();

        virtual ~IConnection();

    protected:
        SOCKET Socket_ = ~0;
        Address Address_ = {};
    };

    class IOConnection : public IConnection
    {
    public:
        IOConnection() = default;
        IOConnection(IOConnection&& b) noexcept = default;
        IOConnection& operator=(IOConnection&& b) noexcept = default;

        explicit IOConnection(Address address) : IConnection(address) {};

        bool Receive(std::vector<char>& buf) const;
        bool Send(std::span<const char> buf) const;
    };

    struct MessageHeader
    {
        MessageHeader() = default;
        explicit MessageHeader(size_t size) : SizeChecksum(CalculateChecksum(size)), Size(size) {};

        [[nodiscard]] bool IsValid() const
        {
            return SizeChecksum == CalculateChecksum(Size) &&
                   Size > 0 &&
                   Magic == MagicValue;
        }

        std::array<char, 3> Magic = MagicValue;
        uint8_t SizeChecksum = UINT8_MAX;
        size_t Size = SIZE_MAX;

    private:
        static constexpr uint8_t CalculateChecksum(size_t size) { return size % UINT8_MAX; }
        static constexpr std::array<char, 3> MagicValue { 'S', 'C', 'K' };
    };

    class InvalidHeaderException final : public std::exception {};

    // Server to client connection
    class ClientConnection final : public IOConnection
    {
    public:
        ClientConnection() = default;
        ClientConnection(ClientConnection&& b) noexcept = default;
        ClientConnection& operator=(ClientConnection&& b) noexcept = default;

        friend class Server;

    private:
        explicit ClientConnection(SOCKET socket);
    };

    class ServerCreationException : public std::exception
    {
    public:
        explicit ServerCreationException(const std::string& what) noexcept : What(what) {};
        explicit ServerCreationException(std::string&& what) noexcept : What(std::move(what)) {};
        explicit ServerCreationException(std::string_view what) noexcept : What(what) {};

        const char* what() const noexcept override { return What.c_str(); }

        std::string What;
    };
    class Server : public IConnection
    {
    public:
        explicit Server(uint16_t port);

        void Listen();
        void Join();
        void Close() override;

        const std::list<ClientConnection>& GetClients() const { return _clients; }

        ~Server() override;

    protected:
        [[nodiscard]] ClientConnection Accept(Address address = {}) const;

        virtual void OnConnect(ClientConnection& connection);
        virtual void OnDisconnect(Address address);
        virtual void OnMessage(ClientConnection& connection, std::span<const char> message);

        void SendToClients(std::span<const char> message) const;
        void SendToClientsExcept(std::span<const char>, std::span<Address> except) const;
        void SendToClientsExcept(std::span<const char>, std::span<ClientConnection*> except) const;

    private:
        void GetConnections();
        void GetMessages();

        std::list<ClientConnection> _clients;
        std::mutex _state;
        std::thread _thread;
        volatile bool _shouldClose = false;
    };

    // Client to server connection
    class Client : public IOConnection
    {
    public:
        explicit Client(Address address);

        // Better hope this returns before the object is deleted.
        // Automatically listens when connected.
        void ConnectAsync(uint8_t retryCount, std::chrono::duration<float> retryTime = std::chrono::seconds(1));

        bool Connect();
        void Listen();

        // Joins thread.
        void Join();
        void Close() override;

        [[nodiscard]] bool IsConnected() const { return _isConnected; }

        ~Client() override;

    protected:
        virtual void OnConnect() { std::println("Connected to server."); };
        virtual void OnDisconnect() { std::println("Disconnected from server."); };
        virtual void OnConnectFailure() { std::println("Failed to connect to server."); };
        virtual void OnMessage(std::span<const char> message) {};
        virtual void OnConnectAttempt(uint8_t attempt) { std::println("Attempting to connect to server. Attempt: {}", static_cast<uint32_t>(attempt)); }

    private:
        bool InternalConnect();

        std::mutex _state;
        std::thread _thread;
        volatile bool _isListening = false;
        bool _isConnected = false;
    };
}
