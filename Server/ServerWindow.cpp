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
        _connectionCountLabel("Connections: 0", WindowTransform{ vec2(0.05f), vec2(0.2f, 0.05f) }, *this),
        _messageField(WindowTransform{ vec2(0.05f, 0.15f), vec2(0.2f, 0.05f) }, *this),
        _submitMessageButton("Send", WindowTransform{ vec2(0.3f, 0.15f), vec2(0.3f, 0.05f) }, *this, this, &ServerWindow::SubmitMessageButton)
    {
        SetMinSize(512);
        Listen();
    }

    void ServerWindow::OnConnect(ClientConnection& connection)
    {
        Server::OnConnect(connection);

        std::ignore = connection.Send("Welcome to the server!"sv);

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
