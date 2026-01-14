//
// Created by scion on 1/12/2026.
//

#pragma once

#include <Secretest/Windowing/Window.h>
#include <Secretest/Networking/Socket.h>

namespace Secretest
{
    class ClientWindow final: public Window, public Client
    {
    public:
        explicit ClientWindow(uvec2 size = uvec2(1440, 720), Address address = {});

    protected:
        void OnCommand() override;
        void OnMessage(std::span<const char> message) override;

    private:
        std::string _messages;
        Label _messagesLabel;
    };
}