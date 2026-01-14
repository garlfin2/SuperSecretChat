//
// Created by scion on 1/12/2026.
//

#pragma once

#include <Secretest/Windowing/Window.h>
#include <Secretest/Networking/Socket.h>

namespace Secretest
{
    class ServerWindow final : public Window, public Server
    {
    public:
        explicit ServerWindow(uvec2 size = uvec2(1440, 720), uint16_t port = 3823);

    protected:
        void OnConnect(ClientConnection& connection) override;
        void OnDisconnect(Address address) override;

    private:
        void UpdateClientCountLabel() const;

        void SubmitMessageButton(IWindow& button);

        Label _connectionCountLabel;
        TextField _messageField;
        DelegateButton<ServerWindow> _submitMessageButton;
    };
}
