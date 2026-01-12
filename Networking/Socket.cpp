//
// Created by scion on 1/11/2026.
//

#include "Socket.h"

#include <format>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace Secretest
{
    SocketContext::SocketContext(uint16_t maxConnections)
    {
        WSADATA wsaData
        {
            MAKEWORD(2, 2),
            MAKEWORD(2, 2),
            maxConnections,
            0,
            nullptr,
            "",
            ""
        };

        if(int result = WSAStartup(MAKEWORD(2, 2), &wsaData))
            return;
    }

    SocketContext::~SocketContext()
    {
        WSACleanup();
    }

    Server::Server(uint16_t port) :
        _self(Address(LOCALHOST, port))
    {
        _self.Listen();
    }

    Client::Client(Address address) :
        _self(address)
    {
        _self.Connect();
    }

    Connection::Connection(const Address address) : _address(address)
    {
        _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(_socket == INVALID_SOCKET)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Socket Creation Error", MB_ICONERROR);
            return;
        }
    }

    Connection::Connection(Connection&& b) noexcept
    {
        _socket = b._socket;
        b._socket = INVALID_SOCKET;
    }

    Connection& Connection::operator=(Connection&& b) noexcept
    {
        if(&b == this)
            return *this;

        this->~Connection();
        _socket = b._socket;
        b._socket = INVALID_SOCKET;

        return *this;
    }

    void Connection::Listen()
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = _address.Port;
        hint.sin_addr.S_un.S_addr = _address.IP;

        if(bind(_socket, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)) == SOCKET_ERROR)
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Binding Error", MB_ICONERROR);

        if(listen(_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Listening Error", MB_ICONERROR);
            return;
        }

        Connection connection = Accept();

        connection.Close();
    }

    void Connection::Connect()
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = _address.Port;
        hint.sin_addr.S_un.S_addr = _address.IP;

        if(connect(_socket, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)) == SOCKET_ERROR)
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Connection Error", MB_ICONERROR);
    }

    void Connection::Close()
    {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }

    Connection Connection::Accept()
    {
        Connection result{};
        result._socket = accept(_socket, nullptr, nullptr);
        return result;
    }

    Connection::~Connection()
    {
        Close();
    }
}

