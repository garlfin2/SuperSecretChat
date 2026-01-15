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
#include <bits/ranges_algo.h>

using namespace std::string_view_literals;

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
            throw ConnectionCreationException();
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

    bool IOConnection::Receive(std::vector<char>& buf) const
    {
        MessageHeader header;
        bool isOpen = recv(Socket_, reinterpret_cast<char*>(&header), sizeof(header), MSG_WAITALL) > 0;

        if(!isOpen || !header.IsValid()) return false;

        buf.resize(header.Size);

        isOpen = recv(Socket_, buf.data(), header.Size, MSG_WAITALL) > 0;

        return isOpen;
    }

    bool IOConnection::Send(std::span<const char> buf) const
    {
        if(buf.size_bytes() == 0)
            return true;

        const MessageHeader header(buf.size_bytes());

        return send(Socket_, reinterpret_cast<const char*>(&header), sizeof(header), 0) > 0 &&
               send(Socket_, buf.data(), buf.size_bytes(), 0) > 0;
    }

    Client::Client(Address address) : IOConnection(address)
    {
    }

    void Client::ConnectAsync(uint8_t retryCount, std::chrono::duration<float> retryTime)
    {
        if(IsConnected())
            return;

        retryCount = std::max<uint8_t>(retryCount, 1);

        std::thread([=, this]()
        {
            for(uint8_t retry = 0; retry < retryCount; retry++)
            {
                InternalConnect();
                if(IsConnected())
                    break;
                std::this_thread::sleep_for(retryTime);
            }

            if(!IsConnected())
                return OnConnectFailure();
            Listen();
        }).detach();
    }

    bool Client::Connect()
    {
        const bool result = InternalConnect();

        if(result)
            Listen();
        else
            OnConnectFailure();

        return result;
    }

    bool Client::InternalConnect()
    {
        if(_isConnected)
            return true;

        sockaddr_in hint{};
        hint.sin_family = AF_INET;
        hint.sin_port = GetAddress().Port;
        hint.sin_addr.S_un.S_addr = GetAddress().IP;

        _isConnected = connect(Socket_, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)) != SOCKET_ERROR;

        if(_isConnected)
            OnConnect();

        return _isConnected;
    }

    void Client::Listen()
    {
        if(_isListening)
            return;

        _thread = std::thread([this]()
        {
            {
                std::unique_lock l{_state};
                _isListening = true;
            }

            std::vector<char> socketBuffer;

            bool isListening;
            do
            {
                {
                    std::unique_lock l{_state};
                    isListening = !_isListening;
                }

                if(!GetStatus(IConnectionStatusQueryType::Read))
                    continue;

                if(!IsOpen() || !Receive(socketBuffer))
                {
                    OnDisconnect();
                    Close();
                    return;
                }

                OnMessage(socketBuffer);
            } while (isListening);
        });
        _thread.detach();
    }

    void Client::Join()
    {
        {
            std::scoped_lock lock{ _state };
            _isListening = false;
        }

        if(_thread.joinable())
            _thread.join();
    }

    void Client::Close()
    {
        Join();

        IConnection::Close();
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

        if(int err = bind(Socket_, reinterpret_cast<const sockaddr*>(&hint), sizeof(sockaddr_in)); err == SOCKET_ERROR)
            throw ServerCreationException(std::format("Failed to listen on socket; is there a server already listening? Code: {}", err));

        if(int err = listen(Socket_, SOMAXCONN); err == SOCKET_ERROR)
            throw ServerCreationException(std::format("Failed to start listening on socket. Code: {}", err));
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

    void Server::OnConnect(ClientConnection& connection) {}

    void Server::OnMessage(ClientConnection& connection, std::span<const char> data) {}

    void Server::OnDisconnect(Address address) {}

    void Server::SendToClients(std::span<const char> message) const
    {
        for(auto& client : _clients)
            client.Send(message);
    }

    void Server::SendToClientsExcept(std::span<const char> message, std::span<Address> except) const
    {
        for(auto& client : _clients)
            if(!std::ranges::contains(except, client.GetAddress()))
                client.Send(message);
    }

    void Server::SendToClientsExcept(std::span<const char> message, std::span<ClientConnection*> except) const
    {
        for(auto& client : _clients)
            if(!std::ranges::contains(except, &client))
                client.Send(message);
    }

    void Server::GetConnections()
    {
        while(GetStatus(IConnectionStatusQueryType::Read) > 0)
        {
            ClientConnection incoming = Accept();
            _clients.push_back(std::move(incoming));
            OnConnect(_clients.back());
        }
    }

    void Server::GetMessages()
    {
        std::vector<char> socketBuffer;

        for(auto client = _clients.begin(); client != _clients.end();)
        {
            if(!client->GetStatus(IConnectionStatusQueryType::Read))
                continue;

            if(!client->IsOpen() || !client->Receive(socketBuffer))
            {
                const Address deleted = client->GetAddress();
                client->Close();
                _clients.erase(client++);
                OnDisconnect(deleted);
                continue;
            }

            OnMessage(*client, socketBuffer);

            ++client;
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

            volatile bool close;
            do
            {
                std::unique_lock l{_state};

                close = _shouldClose;

                GetConnections();
                GetMessages();
            } while(!close);
        });
        _thread.detach();
    }

    void Server::Join()
    {
        {
            std::scoped_lock lock{ _state };
            _shouldClose = true;
        }

        if(_thread.joinable())
            _thread.join();

        for(auto& client : _clients)
            client.Close();
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
