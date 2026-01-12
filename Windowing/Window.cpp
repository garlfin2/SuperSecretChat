//
// Created by scion on 1/10/2026.
//

#include "Window.h"

#include <windows.h>
#include <winuser.h>
#include <unordered_map>

namespace Secretest
{
    LRESULT IWindow::WindowProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam)
    {
        if(auto* obj = reinterpret_cast<IWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
               obj->ProcessInput(uMSG, wParam, lParam);
        return DefWindowProcA(hwnd, uMSG, wParam, lParam);
    }

    void TextField::OnCommand()
    {
        const int length = GetWindowTextLengthA(GetHWND());
        _text = std::string(length, '\0');
        GetWindowTextA(GetHWND(), _text.data(), length + 1);
    }

    void Label::SetText(std::string_view text)
    {
        SetWindowTextA(GetHWND(), text.data());
    }

    Window::Window(uvec2 size): IWindow(IWindowType::Window, "Secretest Chat", uvec2(0x80000000), size),
        _button("Test", uvec2(), uvec2(128, 128), [](const IWindow& w)
        {
            auto size = w.GetWindowSize();
            MessageBox(nullptr, std::format("Button Size: ({}, {})", size.x, size.y).data(), "", MB_OK);
        }, *this),
        _textField(uvec2(0, 300), uvec2(128, 32), *this),
        _testLabel("Test label!", uvec2(0, 500), uvec2(128, 32), *this)
    {
    }

    std::string ToString(const IWindowType e)
    {
        static std::unordered_map<IWindowType, std::string> map
        {
            { IWindowType::Window, "Window" },
            { IWindowType::Button, "Button" },
            { IWindowType::Label, "Static" },
            { IWindowType::TextBox, "Edit" }
        };

        return map[e];
    }

    long ToInternalType(IWindowType e)
    {
        static std::unordered_map<IWindowType, long> map
        {
            { IWindowType::Window, WS_OVERLAPPEDWINDOW },
            { IWindowType::Button, BS_FLAT },
            { IWindowType::Label, 0 },
            { IWindowType::TextBox, WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE }
        };

        return map[e];
    }

    IWindow::IWindow(IWindowType type, std::string_view text, uvec2 position, uvec2 size, const IWindow* parent) :
        _parent(parent),
        _size(size)
    {
        RegisterDefaultStyles();

        HINSTANCE hInstance = GetModuleHandle(nullptr);
        DWORD style = WS_VISIBLE | ToInternalType(type);
        if(_parent)
            style |= WS_CHILD;

        _hwnd = CreateWindowEx(
                    0,
                    ToString(type).c_str(),
                    text.data(),
                    style,
                    position.x,
                    position.y,
                    size.x,
                    size.y,
                    _parent ? _parent->_hwnd : nullptr,
                    nullptr,
                    hInstance,
                    this
                );

        SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    IWindow::~IWindow()
    {
        DestroyWindow(_hwnd);
        _hwnd = nullptr;
    }

    void IWindow::RunWindows()
    {
        MSG msg{};
        while (GetMessage(&msg, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void IWindow::RegisterDefaultStyles()
    {
        static bool wasRegistered = false;
        if(wasRegistered) return;

        RegisterStyle(IWindowType::Window, WindowClass{  });

        wasRegistered = true;
    }

    void IWindow::RegisterStyle(const IWindowType type, const WindowClass& windowClass)
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        const std::string& typeName = ToString(type);

        WNDCLASS wndclass{};
        wndclass.lpszClassName = typeName.c_str();
        wndclass.hInstance = hInstance;
        wndclass.lpfnWndProc = WindowProc;

        RegisterClass(&wndclass);
    }

    void IWindow::ProcessInput(uint message, uint64_t wideParam, uint32_t param)
    {
        IWindow* window;
        switch(message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            _size.x = LOWORD(wideParam);
            _size.y = HIWORD(wideParam);
            break;
        case WM_PAINT:
            Paint();
            break;
        case WM_COMMAND:
            window = reinterpret_cast<IWindow*>(GetWindowLongPtr(reinterpret_cast<HWND>(param), GWLP_USERDATA));
            if(window)
                window->OnCommand();
            break;
        default:
            break;
        }
    }
}
