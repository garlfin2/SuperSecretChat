//
// Created by scion on 1/12/2026.
//

#pragma once

#include <Secretest/Windowing/Window.h>
#include <Secretest/Networking/Socket.h>

namespace Secretest
{
    class ServerWindow final : public IWindow, public Server
    {
    public:
        explicit ServerWindow(uvec2 size = uvec2(1440, 720), uint16_t port = 3823);

    protected:
        void OnCommand() override;
        void OnConnect(ClientConnection& connection) override;
        void OnDisconnect(ClientConnection& connection) override;
    };
}
