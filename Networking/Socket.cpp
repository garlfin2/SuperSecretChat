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

    }

    Connection::Connection(const Address address)
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = address.Port;
        hint.sin_addr.S_un.S_addr = address.IP;

        _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(_socket == INVALID_SOCKET)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Socket Creation Error", MB_ICONERROR);
            return;
        }

        if(bind(_socket, reinterpret_cast<sockaddr*>(&hint), sizeof(sockaddr_in)) == SOCKET_ERROR)
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Binding Error", MB_ICONERROR);
    }

    Connection::Connection(Connection&& b)
    {
        _socket = b._socket;
        b._socket = INVALID_SOCKET;
    }

    Connection& Connection::operator=(Connection&& b)
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
        if(listen(_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Listening Error", MB_ICONERROR);
            return;
        }

        Connection connection = Accept();

        connection.Close();
    }

    void Connection::Close()
    {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }

    Connection Connection::Accept()
    {
        Connection result{};
        accept(result._socket, nullptr, nullptr);
        return result;
    }

    Connection::~Connection()
    {
        closesocket(_socket);
    }
}

