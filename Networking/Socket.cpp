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

        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    SocketContext::~SocketContext()
    {
        WSACleanup();
    }

    ClientConnection Server::Accept(Address address)
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = address.Port;
        hint.sin_addr.S_un.S_addr = address.IP;

        static int hintLength = sizeof(sockaddr_in);

        sockaddr* addr = address.IP ? reinterpret_cast<sockaddr*>(&hint) : nullptr;
        int* addrlen = address.IP ? &hintLength : nullptr;

        ClientConnection result{};
        result.Socket_ = accept(Socket_, addr, addrlen);
        return result;
    }

    IConnection::IConnection(const Address address) : Address_(address)
    {
        Socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(Socket_ == INVALID_SOCKET)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Socket Creation Error", MB_ICONERROR);
            return;
        }
    }

    IConnection::IConnection(IConnection&& b) noexcept
    {
        Socket_ = b.Socket_;
        b.Socket_ = INVALID_SOCKET;
    }

    IConnection& IConnection::operator=(IConnection&& b) noexcept
    {
        if(&b == this)
            return *this;

        Close();
        Socket_ = b.Socket_;
        b.Socket_ = INVALID_SOCKET;

        return *this;
    }

    void IConnection::Close()
    {
        closesocket(Socket_);
        Socket_ = INVALID_SOCKET;
    }

    IConnection::~IConnection()
    {
        Close();
    }

    ClientConnection::ClientConnection(SOCKET socket, Server& server) :
        _server(&server)
    {
        Socket_ = socket;
    }

    void ClientConnection::Join()
    {
        _messageThread.join();
    }

    Client::Client(Address address) : IConnection(address)
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = GetAddress().Port;
        hint.sin_addr.S_un.S_addr = GetAddress().IP;

        if(connect(Socket_, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)) == SOCKET_ERROR)
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Connection Error", MB_ICONERROR);
    }

    Server::Server(uint16_t port) :
       IConnection(Address(LOCALHOST, port))
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = GetAddress().Port;
        hint.sin_addr.S_un.S_addr = GetAddress().IP;

        if(bind(Socket_, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Binding Error", MB_ICONERROR);
            return;
        }

        if(listen(Socket_, SOMAXCONN) == SOCKET_ERROR)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Listening Error", MB_ICONERROR);
            return;
        }
    }

    void Server::Listen()
    {

    }

    void Server::Close()
    {
        {
            std::scoped_lock lock{ _state };
            _clients.clear();
            _shouldRun = false;

            for(ClientConnection& client : _clients)
                client.Join();
        }

        IConnection::Close();
    }
}

