//
// Created by scion on 1/11/2026.
//

#pragma once

#include <cstdint>
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
    };

    enum class IConnectionStatusQueryType
    {
        Read = 1 << 0,
        Write = 1 << 1,
        Exception = 1 << 2
    };

    constexpr uint64_t InvalidSocket = ~0;

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

    class Server;

    // Server to client connection
    class ClientConnection : public IConnection
    {
    public:
        ClientConnection() = default;
        ClientConnection(ClientConnection&& b) noexcept = default;
        ClientConnection& operator=(ClientConnection&& b) noexcept = default;

        bool ReceiveData(std::span<char> buf, size_t& bytesWritten) const;
        bool SendData(std::span<const char> buf) const;

        friend class Server;

    private:
        explicit ClientConnection(SOCKET socket);
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
        ClientConnection Accept(Address address = {}) const;

        virtual void OnConnect(ClientConnection& connection);
        virtual void OnDisconnect(ClientConnection& connection);
        virtual void OnMessage(ClientConnection& connection, std::span<const char> data);

    private:
        void GetConnections();
        void GetMessages();

        std::list<ClientConnection> _clients;
        std::mutex _state;
        std::thread _thread;
        volatile bool _shouldClose = false;
    };

    // Client to server connection
    class Client : public IConnection
    {
    public:
        explicit Client(Address address);

        void Listen();
        void Join();
        void Close() override;

        bool ReceiveData(std::span<char> buf, size_t& bytesWritten) const;
        bool SendData(std::span<const char> buf) const;

        ~Client() override;

    protected:
        virtual void OnConnect();
        virtual void OnDisconnect();
        virtual void OnMessage(std::span<const char> data);

    private:
        std::mutex _state;
        std::thread _thread;
        volatile bool _shouldClose = false;
    };
}
