//
// Created by scion on 1/13/2026.
//

#include "ClientWindow.h"

namespace Secretest
{
    ClientWindow::ClientWindow(uvec2 size, Address address) :
        Window("Secretest", uvec2(), size),
        Client(address)
    {
        SetMinSize(512);

        _connectingStatusWindow = std::make_unique<ConnectingStatusWindow>();

        ConnectAsync(3, std::chrono::duration<float>(1.f));
    }

    void ClientWindow::OnCommand()
    {

    }

    void ClientWindow::PushMessage(std::string_view str)
    {
        _messages.emplace_back(str, WindowTransform{ vec2(_messagePosition), vec2(512, 32), TransformMode::Absolute, TransformMode::Absolute }, *this);
        _messagePosition.y += 32;
    }

    void ClientWindow::UpdateStatusWindow(std::string_view string) const
    {
        _connectingStatusWindow->SetStatusLabel(string);
    }

    void ClientWindow::CloseStatusWindow()
    {
        _connectingStatusWindow = nullptr;
    }

    void ClientWindow::OnMessage(std::span<const char> message)
    {
        Client::OnMessage(message);
        Tasks.Emplace(&ClientWindow::PushMessage, this, std::string(message.begin(), message.end()));
    }

    void ClientWindow::OnConnectAttempt(uint8_t attempt)
    {
        Client::OnConnectAttempt(attempt);
        Tasks.Emplace(&ClientWindow::UpdateStatusWindow, this, std::format("Connecting to server. Attempt: {}", attempt));
    }

    void ClientWindow::OnConnectFailure()
    {
        Client::OnConnectFailure();
        Tasks.Emplace(&ClientWindow::UpdateStatusWindow, this, "Failed to connect to server");
    }

    void ClientWindow::OnConnect()
    {
        Client::OnConnect();
        Tasks.Emplace(&ClientWindow::CloseStatusWindow, this);
    }

    ConnectingStatusWindow::ConnectingStatusWindow() :
        Window("Connecting to Server", uvec2(), uvec2(256), WindowType::Popup),
        _statusLabel("Connecting to server.", WindowTransform(vec2(), vec2(1.f)), *this)
    {

    }
}
