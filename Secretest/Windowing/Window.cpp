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
    WindowTransform WindowTransform::Normalize(vec2 windowSize) const
    {
        WindowTransform result
        {
            Position,
            Size,
            TransformMode::Absolute,
            TransformMode::Absolute
        };

        if(PositionMode.x == TransformMode::Relative) result.Position.x *= windowSize.x;
        if(PositionMode.y == TransformMode::Relative) result.Position.y *= windowSize.y;
        if(ScaleMode.x == TransformMode::Relative) result.Size.x *= windowSize.x;
        if(ScaleMode.y == TransformMode::Relative) result.Size.y *= windowSize.y;

        return result;
    }

    IWindow::IWindow(IWindowType windowStyle, std::string_view text, const WindowTransform& transform, const IWindow* parent) :
        _parent(parent),
        _transform(transform)
    {
        Window::RegisterDefaultStyles();

        HINSTANCE hInstance = GetModuleHandle(nullptr);

        DWORD style = WS_VISIBLE | windowStyle.Flags;
        if(_parent) style |= WS_CHILD;

        const WindowTransform globalTransform = GetGlobalTransform();
        _hwnd = CreateWindowEx(
            0,
            windowStyle.Class.data(),
            text.data(),
            style,
            static_cast<int>(globalTransform.Position.x),
            static_cast<int>(globalTransform.Position.y),
            static_cast<int>(globalTransform.Size.x),
            static_cast<int>(globalTransform.Size.y),
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

        _transform = b._transform;
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

        _transform = b._transform;
        _parent = b._parent;

        SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        return *this;
    }

    void IWindow::RepaintAll() const
    {
        RedrawWindow(GetHWND(), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

    TaskQueue IWindow::Tasks = {};

    TextField::TextField(const WindowTransform& transform, const IWindow& window, std::string_view defaultText):
        IWindow(IWindowType{ "Edit",  TEXT_FIELD_FLAGS }, defaultText, transform, &window),
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

    Label::Label(std::string_view text, const WindowTransform& transform, const IWindow& window):
        IWindow(IWindowType{ "Static", 0 }, text, transform, &window)
    {

    }

    void Label::SetText(std::string_view text) const
    {
        SetWindowTextA(GetHWND(), text.data());
    }

    Window::Window(std::string_view name, uvec2 pos, uvec2 size, WindowType type) :
       IWindow(IWindowType{ type == WindowType::Normal ? "Window" : "Subwindow", static_cast<long>(type) }, name, WindowTransform{vec2(pos), vec2(size), TransformMode::Absolute, TransformMode::Absolute})
    {
    }

    void Window::OnPaint()
    {
        EnumChildWindows(GetHWND(), [](HWND hwnd, LPARAM)
        {
            IWindow* window = reinterpret_cast<IWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            window->OnPaint();

            return 1;
        },0);

        RepaintAll();

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(GetHWND(), &ps);

        FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW));

        EndPaint(GetHWND(), &ps);
    }

    void Window::RunWindows()
    {
        SetTimer(nullptr, 0, 100, nullptr);
        MSG msg{};
        while (GetMessageA(&msg, nullptr, 0, 0))
        {
            Tasks.RunAllTasks();

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    WindowTransform IWindow::GetGlobalTransform() const
    {
        if(_parent)
            return _transform.Normalize(_parent->GetGlobalTransform().Size);
        return _transform.Normalize(vec2(1));
    }

    void IWindow::SetWindowTransform(const WindowTransform& transform)
    {
        _transform = transform;

        UpdateTransform();
    }

    void IWindow::UpdateTransform() const
    {
        const WindowTransform globalTransform = GetGlobalTransform();
        SetWindowPos(
            _hwnd,
            nullptr,
            static_cast<int>(globalTransform.Position.x),
            static_cast<int>(globalTransform.Position.y),
            static_cast<int>(globalTransform.Size.x),
            static_cast<int>(globalTransform.Size.y),
            0
        );
    }

    void IWindow::OnPaint()
    {
        UpdateTransform();

        EnumChildWindows(GetHWND(), [](HWND hwnd, LPARAM)
        {
            IWindow* window = reinterpret_cast<IWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

            window->OnPaint();

            return 1;
        },0);

        RepaintAll();
    }

    LRESULT Window::WindowProc(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam)
    {
        if(auto* obj = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
            obj->ProcessInput(uMSG, wParam, lParam);
        return DefWindowProcA(hwnd, uMSG, wParam, lParam);
    }

    LRESULT Window::WindowProcNoQuit(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam)
    {
        if(uMSG == WM_DESTROY)
            return 1;

        return WindowProc(hwnd, uMSG, wParam, lParam);
    }

    void Window::RegisterDefaultStyles()
    {
        static bool wasRegistered = false;
        if(wasRegistered) return;

        RegisterStyle(IWindowClass{ "Window", WindowProc });
        RegisterStyle(IWindowClass{ "Subwindow", WindowProcNoQuit });

        wasRegistered = true;
    }

    void Window::SetWindowTransform(uvec2 position, uvec2 size)
    {
        IWindow::SetWindowTransform(WindowTransform{
            vec2(position),
            vec2(size),
            TransformMode::Absolute,
            TransformMode::Absolute
        });
    }

    void Window::RegisterStyle(const IWindowClass& windowClass)
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        WNDCLASS wndclass{};
        wndclass.lpszClassName = windowClass.Name.data();
        wndclass.hInstance = hInstance;
        wndclass.lpfnWndProc = windowClass.WindowProc;

        RegisterClass(&wndclass);
    }

    void Window::ProcessInput(uint message, uint64_t wParam, int64_t param)
    {
        IWindow* window = reinterpret_cast<IWindow*>(GetWindowLongPtr(reinterpret_cast<HWND>(param), GWLP_USERDATA));
        LPMINMAXINFO lpMMI = reinterpret_cast<LPMINMAXINFO>(param);

        switch(message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            SetWindowTransformInternal(WindowTransform{
                GetRelativeTransform().Position,
                vec2(LOWORD(param), HIWORD(param)),
                TransformMode::Absolute,
                TransformMode::Absolute
            });
            RepaintAll();
            break;
        case WM_PAINT:
            OnPaint();
            break;
        case WM_COMMAND:
            window->OnCommand();
            break;
        case WM_GETMINMAXINFO:
            lpMMI->ptMinTrackSize = POINT(_minSize.x, _minSize.y);
            lpMMI->ptMaxTrackSize = POINT(_maxSize.x, _maxSize.y);
            break;
        default:
            break;
        }
    }
}
