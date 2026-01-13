//
// Created by scion on 1/13/2026.
//

#include "ServerWindow.h"

namespace Secretest
{
    ServerWindow::ServerWindow(uvec2 size, uint16_t port) :
        IWindow(IWindowType::Window, "Secretest Server", uvec2(), size),
        _server(port)
    {
        _server.Listen();
    }

    void ServerWindow::OnCommand()
    {

    }
}
