//
// Created by scion on 1/10/2026.
//

#pragma once

#include <Secretest/Utility/vec2.h>
#include <functional>
#include <string>
#include <thread>
#include <print>

struct HWND__;
using HWND = HWND__*;
using ATOM = uint16_t;
using LRESULT = int64_t;

namespace Secretest
{
    struct IWindowClass
    {
        std::string_view Class;
        long Flags;
    };

    class IWindow
    {
    public:
        IWindow(IWindowClass windowClass, std::string_view name, uvec2 position, uvec2 size, const IWindow* parent = nullptr);
        virtual ~IWindow();

        IWindow(IWindow&&) = default;
        IWindow& operator=(IWindow&&) = default;

        IWindow(const IWindow&) = delete;
        IWindow& operator=(const IWindow&) = delete;

        static void RunWindows();
        static void RegisterDefaultStyles();

        [[nodiscard]] u16vec2 GetWindowSize() const { return _size; }

    protected:
        virtual void ProcessInput(uint message, uint64_t wideParam, uint32_t param);
        virtual void Paint() {};
        virtual void OnCommand() = 0;

        [[nodiscard]] HWND GetHWND() const { return _hwnd; }

    private:
        static LRESULT WindowProc(HWND, uint, uint64_t, int64_t);

        HWND _hwnd;

        const IWindow* _parent;
        u16vec2 _size;
    };

    template<typename FUNC_T> requires requires(FUNC_T f, IWindow& b) { { f(b) }; }
    class IButton final : public IWindow
    {
    public:
        IButton(std::string_view text, uvec2 pos, uvec2 size, FUNC_T&& func, const IWindow& window) :
            IWindow({ "Button",  }, text, pos, size, &window),
            _func(std::move(func))
        {}

        IButton(std::string_view text, uvec2 pos, uvec2 size, const FUNC_T& func, const IWindow& window) :
            IWindow(IWindowType::Button, text, pos, size, &window),
            _func(func)
        {}

    protected:
        void OnCommand() override { _func(*this); }

    private:
        FUNC_T _func;
    };

    using Button = IButton<std::function<void(IWindow&)>>;

    class TextField final : public IWindow
    {
    public:
        TextField(uvec2 pos, uvec2 size, const IWindow& window, std::string_view defaultText = "") :
            IWindow(IWindowType::TextBox, defaultText, pos, size, &window),
            _text(defaultText)
        {}

        [[nodiscard]] const std::string& GetText() const { return _text; }

    protected:
        void OnCommand() override;

    private:
        std::string _text;
    };

    class Label final : public IWindow
    {
    public:
        Label(const char* text, uvec2 pos, uvec2 size, const IWindow& window) :
            IWindow(IWindowType::Label, text, pos, size, &window)
        {}

        void SetText(std::string_view text);

    protected:
        void OnCommand() override {};
    };

    class Window final :
}