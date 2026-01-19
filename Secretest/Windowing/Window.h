//
// Created by scion on 1/10/2026.
//

#pragma once

#include "Threading.h"

#include <Secretest/Utility/vec2.h>
#include <functional>
#include <minwindef.h>
#include <string>
#include <thread>

using HWND = class HWND__*;
using ATOM = uint16_t;
using LRESULT = int64_t;

namespace Secretest
{
    enum class TransformMode : uint8_t
    {
        Relative,
        Absolute
    };

    struct WindowTransform
    {
        vec2 Position = vec2();
        vec2 Size = vec2(1.f);

        vec<2, TransformMode> PositionMode = TransformMode::Relative;
        vec<2, TransformMode> ScaleMode = TransformMode::Relative;

        [[nodiscard]] WindowTransform Normalize(vec2 windowSize) const;
    };

    struct IWindowType
    {
        std::string_view Class;
        long Flags;
    };

    using WindowProc = LRESULT(*)(HWND, uint, uint64_t, int64_t);
    struct IWindowClass
    {
        std::string_view Name;
        WindowProc WindowProc;
    };

    class IWindow
    {
    public:
        IWindow(IWindowType windowStyle, std::string_view name, const WindowTransform& transform, const IWindow* parent = nullptr);
        virtual ~IWindow();

        IWindow(IWindow&&) noexcept;
        IWindow& operator=(IWindow&&) noexcept;

        IWindow(const IWindow&) = delete;
        IWindow& operator=(const IWindow&) = delete;

        [[nodiscard]] const WindowTransform& GetRelativeTransform() const { return _transform; }
        [[nodiscard]] WindowTransform GetGlobalTransform() const;
        void SetWindowTransform(const WindowTransform& transform);

        void RepaintAll() const;

        friend class Window;

    protected:
        virtual void OnCommand() = 0;
        virtual void OnPaint();
        void SetWindowTransformInternal(const WindowTransform& transform) { _transform = transform; }
        [[nodiscard]] bool IsOpen() const { return _isOpen; }

        static TaskQueue Tasks;

        [[nodiscard]] HWND GetHWND() const { return _hwnd; }

    private:
        void UpdateTransform() const;

        HWND _hwnd;
        bool _isOpen = true;

        const IWindow* _parent;
        WindowTransform _transform;
    };

    template<class FUNC_T>
    concept IsButtonFunction = requires(FUNC_T f, IWindow& b) { { f(b) }; };

    template<typename FUNC_T> requires IsButtonFunction<FUNC_T>
    class IButton final : public IWindow
    {
    public:
        template<typename... ARGS>
        IButton(std::string_view text, const WindowTransform& transform, const IWindow& window, ARGS&&... args) :
            IWindow({ "Button", 0 }, text, transform, &window),
            _func(std::forward<ARGS>(args)...)
        {}

    protected:
        void OnCommand() override { _func(*this); }

    private:
        FUNC_T _func;
    };

    using Button = IButton<std::function<void(IWindow&)>>;

    template<typename DELEGATE_T, typename FUNC_T>
    concept IsButtonDelegate = requires(FUNC_T f, DELEGATE_T* d, IWindow& b) { { f(d, b) }; };

    template<typename DELEGATE_T, typename FUNC_T = std::function<void(DELEGATE_T* d, IWindow& w)>> requires IsButtonDelegate<DELEGATE_T, FUNC_T>
    struct ButtonDelegate
    {
        DELEGATE_T* T;
        FUNC_T Function;

        void operator()(IWindow& window) { Function(T, window); }
    };

    template<typename DELEGATE_T, typename FUNC_T = std::function<void(DELEGATE_T* d, IWindow& w)>>
    using DelegateButton = IButton<ButtonDelegate<DELEGATE_T, FUNC_T>>;

    class TextField final : public IWindow
    {
    public:
        TextField(const WindowTransform& transform, const IWindow& window, std::string_view defaultText = "");

        [[nodiscard]] std::string_view GetText() const { return _text; }
        void SetText(std::string_view text);
        void ClearText() { SetText(""); }

    protected:
        void OnCommand() override;

    private:
        std::string _text;
    };

    class Label final : public IWindow
    {
    public:
        Label(std::string_view text, const WindowTransform& transform, const IWindow& window);

        void SetText(std::string_view text) const;

    protected:
        void OnCommand() override {};
    };

    enum class WindowType : uint32_t
    {
        Normal = 0x00CF0000l,
        Popup = 0x80880000l,
    };

    class Window : public IWindow
    {
    public:
        Window(std::string_view name, uvec2 pos, uvec2 size, WindowType type = WindowType::Normal);

        static void RunWindows();
        static void RegisterStyle(const IWindowClass& windowClass);
        static void RegisterDefaultStyles();

        void SetWindowTransform(uvec2 position, uvec2 size);

        void SetMinMaxSize(uvec2 min, uvec2 max) { _minSize = min; _maxSize = max; }
        void SetMinSize(uvec2 min) { _minSize = min; }
        void SetMaxSize(uvec2 max) { _maxSize = max; }

    protected:
        virtual void ProcessInput(uint message, uint64_t wParam, int64_t param);
        void OnPaint() override;
        void OnCommand() override {};

    private:
        static LRESULT WindowProc(HWND, uint, uint64_t, int64_t);
        static LRESULT WindowProcNoQuit(HWND hwnd, UINT uMSG, WPARAM wParam, LPARAM lParam);

        using IWindow::SetWindowTransform;

        uvec2 _minSize = 0;
        uvec2 _maxSize = INT32_MAX;
    };
}