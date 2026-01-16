//
// Created by scion on 1/10/2026.
//

#include "Window.h"

#include <windows.h>
#include <winuser.h>
#include <unordered_map>

#define TEXT_FIELD_FLAGS WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE

namespace Secretest
{
    LRESULT IWindow::WindowProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam)
    {
        if(auto* obj = reinterpret_cast<IWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
               obj->ProcessInput(uMSG, wParam, lParam);
        return DefWindowProcA(hwnd, uMSG, wParam, lParam);
    }

    TextField::TextField(uvec2 pos, uvec2 size, const IWindow& window, std::string_view defaultText):
        IWindow(IWindowType{ "Edit",  TEXT_FIELD_FLAGS }, defaultText, pos, size, &window),
        _text(defaultText)
    {

    }

    void TextField::SetText(std::string_view text)
    {
        SetWindowTextA(GetHWND(), text.data());
        _text = text;
    }

    void TextField::OnCommand()
    {
        const int length = GetWindowTextLengthA(GetHWND());
        _text = std::string(length, '\0');
        GetWindowTextA(GetHWND(), _text.data(), length + 1);
    }

    Label::Label(std::string_view text, uvec2 pos, uvec2 size, const IWindow& window):
        IWindow(IWindowType{ "Static", 0 }, text, pos, size, &window)
    {

    }

    void Label::SetText(std::string_view text) const
    {
        SetWindowTextA(GetHWND(), text.data());
    }

    Window::Window(std::string_view name, uvec2 position, uvec2 size) :
        IWindow(IWindowType{ "Window", WS_OVERLAPPEDWINDOW }, name, position, size)
    {
    }

    void Window::RepaintAll() const
    {
        RedrawWindow(GetHWND(), nullptr, nullptr, RDW_INTERNALPAINT);
    }

    void Window::Paint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(GetHWND(), &ps);

        FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW));

        EndPaint(GetHWND(), &ps);
    }

    IWindow::IWindow(IWindowType windowStyle, std::string_view text, uvec2 position, uvec2 size, const IWindow* parent) :
        _parent(parent),
        _size(size)
    {
        RegisterDefaultStyles();

        HINSTANCE hInstance = GetModuleHandle(nullptr);

        DWORD style = WS_VISIBLE | windowStyle.Flags;
        if(_parent) style |= WS_CHILD;

        _hwnd = CreateWindowEx(
            0,
            windowStyle.Class.data(),
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

    IWindow::IWindow(IWindow&& b) noexcept
    {
        _hwnd = b._hwnd;
        b._hwnd = nullptr;

        _size = b._size;
        _parent = b._parent;

        SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    IWindow& IWindow::operator=(IWindow&& b) noexcept
    {
        if(&b == this)
            return *this;

        this->~IWindow();

        _hwnd = b._hwnd;
        b._hwnd = nullptr;

        _size = b._size;
        _parent = b._parent;

        SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        return *this;
    }

    TaskQueue IWindow::Tasks = {};

    void IWindow::RunWindows()
    {
        MSG msg{};
        while (GetMessageA(&msg, nullptr, 0, 0))
        {
            Tasks.RunAllTasks();

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void IWindow::RegisterDefaultStyles()
    {
        static bool wasRegistered = false;
        if(wasRegistered) return;

        RegisterStyle(IWindowClass{ "Window" });

        wasRegistered = true;
    }

    void IWindow::RegisterStyle(const IWindowClass& windowClass)
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        WNDCLASS wndclass{};
        wndclass.lpszClassName = windowClass.Name.data();
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
