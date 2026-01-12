//
// Created by scion on 1/11/2026.
//

#pragma once
#include <cstdint>
#include <mutex>
#include <vector>

struct addrinfo;
using SOCKET = uint64_t;
using WSAEVENT = void*;

#define LOCALHOST 0x0100007F

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

        union
        {
            uint32_t IP;
            uint8_t IPBytes[4];
        };

        uint16_t Port;
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

        void Close();

        ~IConnection();

    protected:
        SOCKET Socket_ = ~0;
        Address Address_ = {};
    };

    class Server;

    // Server to client connection
    class ClientConnection : private IConnection
    {
    public:
        ClientConnection();
        ClientConnection(SOCKET socket, Server& server);

        friend class Server;

    private:
        void Join();
        void Listen();

        std::thread _messageThread;
        std::mutex _state;
        Server* _server = nullptr;
    };

    class Server : private IConnection
    {
    public:
        explicit Server(uint16_t port);
        virtual ~Server() = default;

        void Listen();
        void Close();

    protected:
        ClientConnection Accept(Address address = {});

        virtual void OnConnectionRequest();
        virtual void OnConnectionAccept();
        virtual void OnDisconnect();
        virtual void OnMessage();

    private:
        std::vector<ClientConnection> _clients;
        std::mutex _state;
        bool _shouldRun = true;
    };

    // Client to server connection
    class Client : public  IConnection
    {
    public:
        explicit Client(Address address);
        virtual ~Client() = default;
    };
}
