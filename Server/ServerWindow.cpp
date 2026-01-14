//
// Created by scion on 1/13/2026.
//

#include "ServerWindow.h"

#include <sstream>

using namespace std::string_view_literals;

namespace Secretest
{
    ServerWindow::ServerWindow(uvec2 size, uint16_t port) :
        Window("Secretest Server", uvec2(), size),
        Server(port),
        _connectionCountLabel("Connections: 0", uvec2(32, 32), uvec2(128, 32), *this),
        _messageField(uvec2(32, 160), uvec2(128, 32), *this),
        _submitMessageButton("Send", uvec2(192, 160), uvec2(128, 32), *this, this, &ServerWindow::SubmitMessageButton)
    {
        Listen();
    }

    void ServerWindow::OnConnect(ClientConnection& connection)
    {
        Server::OnConnect(connection);

        connection.Send("Welcome to the server!"sv);

        UpdateClientCountLabel();
    }

    void ServerWindow::OnDisconnect(Address address)
    {
        Server::OnDisconnect(address);

        UpdateClientCountLabel();
    }

    void ServerWindow::UpdateClientCountLabel() const
    {
        _connectionCountLabel.SetText(std::format("Connections: {}", GetClients().size()));
    }

    void ServerWindow::SubmitMessageButton(IWindow& button)
    {
        SendToClients(_messageField.GetText());
        _messageField.ClearText();
    }
}
