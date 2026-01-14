//
// Created by scion on 1/13/2026.
//

#include "ClientWindow.h"

namespace Secretest
{
    ClientWindow::ClientWindow(uvec2 size, Address address) :
        Window("Secretest", uvec2(), size),
        Client(address),
        _messagesLabel("", uvec2(32), uvec2(512), *this)
    {
        Listen();
    }

    void ClientWindow::OnCommand()
    {

    }

    void ClientWindow::OnMessage(std::span<const char> message)
    {
        Client::OnMessage(message);

        _messages += static_cast<std::string_view>(message);
        _messages += '\n';
        _messagesLabel.SetText(_messages);
    }
}
