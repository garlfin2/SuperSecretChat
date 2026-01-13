//
// Created by scion on 1/11/2026.
//

#include "Socket.h"

#include <Secretest/Utility/Enum.h>
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

    IConnection::IConnection(const Address address) : Address_(address)
    {
        Socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(Socket_ == InvalidSocket)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Socket Creation Error", MB_ICONERROR);
            return;
        }
    }

    IConnection::IConnection(IConnection&& b) noexcept
    {
        Socket_ = b.Socket_;
        Address_ = b.Address_;
        b.Socket_ = InvalidSocket;
    }

    IConnection& IConnection::operator=(IConnection&& b) noexcept
    {
        if(&b == this)
            return *this;

        Close();
        Socket_ = b.Socket_;
        Address_ = b.Address_;
        b.Socket_ = InvalidSocket;

        return *this;
    }

    int32_t IConnection::GetStatus(IConnectionStatusQueryType type) const
    {
        static constexpr timeval doNotBlock{ 0, 0 };

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
        Socket_ = InvalidSocket;
    }

    IConnection::~IConnection()
    {
        IConnection::Close();
    }

    ClientConnection::ClientConnection(SOCKET socket)
    {
        Socket_ = socket;

        sockaddr_in address;
        int addrSize = sizeof(address);

        getpeername(socket, reinterpret_cast<sockaddr*>(&address), &addrSize);

        Address_ = Address(address.sin_addr.S_un.S_addr, address.sin_port);
    }

    bool ClientConnection::ReceiveData(std::span<char> buf, size_t& bytesWritten) const
    {
        bytesWritten = recv(Socket_, buf.data(), buf.size_bytes(), 0);
        return bytesWritten > 0;
    }

    bool ClientConnection::SendData(std::span<const char> buf) const
    {
        return send(Socket_, buf.data(), buf.size_bytes(), 0) > 0;
    }

    Client::Client(Address address) : IConnection(address)
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = GetAddress().Port;
        hint.sin_addr.S_un.S_addr = GetAddress().IP;

        if(connect(Socket_, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            MessageBox(nullptr, std::format("Error code {}", WSAGetLastError()).c_str(), "Connection Error", MB_ICONERROR);
            return;
        }
    }

    void Client::Listen()
    {
        _thread = std::thread([this]()
        {
            {
                std::unique_lock l{_state};
                _shouldClose = false;
            }

            std::array<char, SOCKET_BUF_SIZE> socketBuffer;

            while(!_shouldClose)
            {
                std::unique_lock l{_state};

                if(!GetStatus(IConnectionStatusQueryType::Read))
                    continue;

                size_t bytesRead;
                if(!IsOpen() || !ReceiveData(socketBuffer, bytesRead))
                {
                    OnDisconnect();
                    Close();
                    return;
                }

                OnMessage(std::span(socketBuffer.begin(), socketBuffer.begin() + bytesRead));
            }
        });
        _thread.detach();
    }

    void Client::Join()
    {
        {
            std::scoped_lock lock{ _state };
            _shouldClose = true;
        }

        if(_thread.joinable())
            _thread.join();
    }

    void Client::Close()
    {
        Join();

        IConnection::Close();
    }

    bool Client::SendData(std::span<const char> buf) const
    {
        return send(Socket_, buf.data(), buf.size_bytes(), 0) > 0;
    }

    void Client::OnConnect()
    {

    }

    void Client::OnDisconnect()
    {

    }

    void Client::OnMessage(std::span<const char> data)
    {
        MessageBoxA(nullptr, data.data(), "Message Received", MB_ICONINFORMATION);
    }

    bool Client::ReceiveData(std::span<char> buf, size_t& bytesWritten) const
    {
        bytesWritten = recv(Socket_, buf.data(), buf.size_bytes(), 0);
        return bytesWritten > 0;
    }

    Client::~Client()
    {
        Join();
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

    ClientConnection Server::Accept(Address address) const
    {
        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = address.Port;
        hint.sin_addr.S_un.S_addr = address.IP;

        static int hintLength = sizeof(sockaddr_in);

        sockaddr* addr = address.IP ? reinterpret_cast<sockaddr*>(&hint) : nullptr;
        int* addrSize = address.IP ? &hintLength : nullptr;

        return ClientConnection(accept(Socket_, addr, addrSize));
    }

    void Server::OnConnect(ClientConnection& connection)
    {
        const std::string ip = static_cast<std::string>(connection.GetAddress());
        MessageBoxA(nullptr, ip.c_str(), "Client Connected", MB_OK);

        static std::string_view coolMessage = "Welcome to the server!";

        connection.SendData(coolMessage);
    }

    void Server::OnMessage(ClientConnection& connection, std::span<const char> data)
    {

    }

    void Server::OnDisconnect(ClientConnection& connection)
    {
        const std::string ip = static_cast<std::string>(connection.GetAddress());
        MessageBoxA(nullptr, ip.c_str(), "Client Disconnected", MB_OK);
    }

    void Server::GetConnections()
    {
        std::unique_lock l{_state};

        while(GetStatus(IConnectionStatusQueryType::Read) > 0)
        {
            ClientConnection incoming = Accept();
            OnConnect(incoming);
            _clients.push_back(std::move(incoming));
        }
    }

    void Server::GetMessages(bool& requiresCleanup)
    {
        std::unique_lock l{_state};

        std::array<char, SOCKET_BUF_SIZE> socketBuffer;

        for(ClientConnection& client : _clients)
        {
            if(!client.GetStatus(IConnectionStatusQueryType::Read))
                continue;

            size_t bytesRead;
            if(!client.IsOpen() || !client.ReceiveData(socketBuffer, bytesRead))
            {
                requiresCleanup = true;
                OnDisconnect(client);
                client.Close();
                continue;
            }

            OnMessage(client, std::span(socketBuffer.begin(), socketBuffer.begin() + bytesRead));
        }
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

                bool requiresCleanup;

                GetConnections();
                GetMessages(requiresCleanup);

                if(requiresCleanup)
                    std::erase_if(_clients, [](const ClientConnection& client){ return !client.IsOpen(); });
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

        for(auto& client : _clients)
            client.Close();

        if(_thread.joinable())
            _thread.join();
    }

    void Server::Close()
    {
        Join();

        _clients.clear();

        IConnection::Close();
    }

    Server::~Server()
    {
        Join();
    }
}