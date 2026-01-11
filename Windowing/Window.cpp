//
// Created by scion on 1/10/2026.
//

#include "Window.h"

#include <windows.h>
#include <winuser.h>
#include <unordered_map>

namespace Discord
{
    LRESULT IWindow::WindowProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam)
    {
        IWindow* obj = nullptr;

        if(uMSG != WM_NCCREATE)
            obj = reinterpret_cast<IWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        if(obj)
               obj->ProcessInput(uMSG, wParam, lParam);
        return DefWindowProcA(hwnd, uMSG, wParam, lParam);
    }

    std::string ToString(const IWindowType e)
    {
        static std::unordered_map<IWindowType, std::string> map
        {
            { IWindowType::Window, "Window" },
            { IWindowType::Button, "Button" },
            { IWindowType::Label, "Static" }
        };

        return map[e];
    }

    IWindow::IWindow(IWindowType type, const std::string& text, uvec2 position, uvec2 size, const IWindow* parent) :
        _parent(parent),
        _text(text),
        _size(size)
    {
        RegisterDefaultStyles();

        HINSTANCE hInstance = GetModuleHandle(nullptr);
        DWORD style = WS_VISIBLE | static_cast<DWORD>(type);
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
                window->OnClick();
            break;
        default:
            break;
        }
    }
}
