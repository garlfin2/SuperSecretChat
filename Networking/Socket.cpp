//
// Created by scion on 1/11/2026.
//

#include "Socket.h"
#include "Utility/Enum.h"

#include <format>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <print>

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

    std::unique_ptr<ClientConnection> Server::Accept(Address address)
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = address.Port;
        hint.sin_addr.S_un.S_addr = address.IP;

        static int hintLength = sizeof(sockaddr_in);

        sockaddr* addr = address.IP ? reinterpret_cast<sockaddr*>(&hint) : nullptr;
        int* addrSize = address.IP ? &hintLength : nullptr;

        const SOCKET socket = accept(Socket_, addr, addrSize);
        if(socket != INVALID_SOCKET)
            return std::make_unique<ClientConnection>(socket, *this);
        return {};
    }

    void Server::OnConnection(ClientConnection& connection)
    {
        std::string ip = static_cast<std::string>(connection.GetAddress());
        MessageBoxA(nullptr, ip.c_str(), "New Connection!", MB_OK);
    }

    void Server::OnMessage(ClientConnection& connection, const std::vector<uint8_t>& data)
    {

    }

    void Server::OnDisconnect(ClientConnection& connection)
    {

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

    int32_t IConnection::GetStatus(IConnectionStatusQueryType type) const
    {
        const static timeval doNotBlock{ 0, 0 };

        fd_set set{ 1, { Socket_ } };

        return select(
            0,
            CheckEnumFlags(type, IConnectionStatusQueryType::Read) ? &set : nullptr,
            CheckEnumFlags(type, IConnectionStatusQueryType::Write) ? &set : nullptr,
            CheckEnumFlags(type, IConnectionStatusQueryType::Exception) ? &set : nullptr,
            &doNotBlock
        );
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

        sockaddr_in address;
        static int addrSize = sizeof(address);

        getpeername(socket, reinterpret_cast<sockaddr*>(&address), &addrSize);

        Address_ = Address(address.sin_addr.S_un.S_addr, address.sin_port);
    }

    ClientConnection::~ClientConnection()
    {
        Close();
    }

    void ClientConnection::Listen()
    {
        _messageThread = std::thread([this]()
        {
            {
                std::unique_lock l{_state};
                _shouldClose = false;
            }
            char buf[SOCKET_BUF_LENGTH];
            while(recv(Socket_, buf, sizeof(buf), 0) > 0 && !_shouldClose)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        _messageThread.detach();
    }

    void ClientConnection::Join()
    {
        {
            std::unique_lock l{_state};
            _shouldClose = true;
        }
        if(_messageThread.joinable())
            _messageThread.join();
    }

    void ClientConnection::Close()
    {
        Join();
        IConnection::Close();
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

    Server::~Server()
    {
        Close();
    }

    void Server::Listen()
    {
        _thread = std::thread([this]()
        {
            {
                std::unique_lock l{_state};
                _shouldClose = false;
            }

            while(!_shouldClose)
            {
                std::unique_lock l{_state};

                std::unique_ptr<ClientConnection> incoming;
                if(GetStatus(IConnectionStatusQueryType::Read) > 0 && ((incoming = Accept())))
                {
                    OnConnection(*incoming);
                    incoming->Listen();
                    _clients.push_back(std::move(incoming));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        _thread.detach();
    }

    void Server::Join()
    {
        {
            std::scoped_lock lock{ _state };
            _shouldClose = true;
        }

        for(const auto& client : _clients)
            client->Join();
    }

    void Server::Close()
    {
        Join();

        _clients.clear();

        IConnection::Close();
    }
}