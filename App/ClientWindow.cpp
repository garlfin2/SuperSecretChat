//
// Created by scion on 1/13/2026.
//

#include "ClientWindow.h"

namespace Secretest
{
    ClientWindow::ClientWindow(uvec2 size, Address address) :
        IWindow(IWindowType::Window, "Secretest", uvec2(), size),
        _client(address)
    {
        _client.Listen();
    }

    void ClientWindow::OnCommand()
    {

    }
}
