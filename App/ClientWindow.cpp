//
// Created by scion on 1/13/2026.
//

#include "ClientWindow.h"

namespace Secretest
{
    ClientWindow::ClientWindow(uvec2 size, Address address) :
        Window("Secretest", uvec2(), size),
        Client(address)
    {
        Listen();
    }

    void ClientWindow::OnCommand()
    {

    }

    void ClientWindow::PushMessage(const std::string& str)
    {
        _messages.emplace_back(str, _messagePosition, uvec2(512, 32), *this);
        _messagePosition.y += 32;
    }

    void ClientWindow::OnMessage(std::span<const char> message)
    {
        Tasks.Emplace(&ClientWindow::PushMessage, this, std::string(message.begin(), message.end()));
        Client::OnMessage(message);
    }
}
