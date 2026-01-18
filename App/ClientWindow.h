//
// Created by scion on 1/12/2026.
//

#pragma once

#include <Secretest/Windowing/Window.h>
#include <Secretest/Networking/Socket.h>

namespace Secretest
{
    class ConnectingStatusWindow final : public Window
    {
    public:
        ConnectingStatusWindow();

        void SetStatusLabel(std::string_view& string) const { _statusLabel.SetText(string); }

    private:
        Label _statusLabel;
    };

    class ClientWindow final : public Window, public Client
    {
    public:
        explicit ClientWindow(uvec2 size = uvec2(1440, 720), Address address = {});

    protected:
        void OnCommand() override;
        void OnConnect() override;
        void OnMessage(std::span<const char> message) override;
        void OnConnectAttempt(uint8_t attempt) override;
        void OnConnectFailure() override;

        void PushMessage(std::string_view str);
        void UpdateStatusWindow(std::string_view string) const;
        void CloseStatusWindow();

    private:
        std::list<Label> _messages;
        uvec2 _messagePosition{ 32, 32 };

        std::unique_ptr<ConnectingStatusWindow> _connectingStatusWindow;
    };
}