//
// Created by scion on 1/11/2026.
//

#pragma once
#include <cstdint>

struct addrinfo;
using SOCKET = uint64_t;

#define LOCALHOST 0

namespace Secretest
{
    class SocketContext
    {
    public:
        SocketContext(uint16_t maxConnections = 32);
        ~SocketContext();
    };

    struct Address
    {
        Address(uint32_t ip, uint16_t port) : IP(ip), Port(port) {};
        Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port) : IPBytes{ a, b, c, d }, Port(port) {};

        union
        {
            uint32_t IP;
            uint8_t IPBytes[4];
        };

        uint16_t Port;
    };

    class Connection
    {
    public:
        Connection(Address address);
        Connection() = default;

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        Connection(Connection&& b);
        Connection& operator=(Connection&& b);

        void Listen();
        void Close();
        Connection Accept();

        ~Connection();

        explicit operator SOCKET() const { return _socket; }

    private:
        SOCKET _socket = ~0;
    };

    class Server
    {
    public:
        explicit Server(uint16_t port);
        virtual ~Server() = default;

        virtual void OnMessage() {};

    private:
        Connection _self;
    };

    class Client
    {
    public:
        explicit Client(Address address);
        virtual ~Client() = default;

    private:
        Connection _self;
    };
}
