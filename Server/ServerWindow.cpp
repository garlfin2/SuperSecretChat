//
// Created by scion on 1/13/2026.
//

#include "ServerWindow.h"

namespace Secretest
{
    ServerWindow::ServerWindow(uvec2 size, uint16_t port) :
        Window("Secretest Server", uvec2(), size),
        Server(port),
        _connectionCountLabel("Connections: 0", uvec2(0), uvec2(128, 128), *this)
    {
        Listen();
    }

    void ServerWindow::OnConnect(ClientConnection& connection)
    {
        Server::OnConnect(connection);

        UpdateClientCountLabel();
    }

    void ServerWindow::OnDisconnect(ClientConnection& connection)
    {
        Server::OnDisconnect(connection);

        UpdateClientCountLabel();
    }

    void ServerWindow::UpdateClientCountLabel() const
    {
        _connectionCountLabel.SetText(std::format("Connections: {}", GetClients().size()));
    }
}
