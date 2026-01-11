//
// Created by scion on 1/10/2026.
//

#pragma once

#include <functional>
#include <string>
#include <thread>
#include <Utility/vec2.h>
#include <print>

struct HWND__;
using HWND = HWND__*;
using ATOM = uint16_t;
using LRESULT = int64_t;

namespace Discord
{
    enum class IWindowType
    {
        Window = 0x00CF0000,
        Button = 0,
        Label,
    };

    std::string ToString(IWindowType e);

    // TODO
    struct WindowClass { };

    class IWindow
    {
    public:
        IWindow(IWindowType type, const std::string& name, uvec2 position, uvec2 size, const IWindow* parent = nullptr);
        virtual ~IWindow();

        IWindow(IWindow&&) = default;
        IWindow& operator=(IWindow&&) = default;

        IWindow(const IWindow&) = delete;
        IWindow& operator=(const IWindow&) = delete;

        static void RunWindows();
        static void RegisterDefaultStyles();
        static void RegisterStyle(IWindowType type, const WindowClass& windowClass);

        [[nodiscard]] const std::string& GetWindowText() const { return _text; }
        [[nodiscard]] u16vec2 GetWindowSize() const { return _size; }

    protected:
        virtual void ProcessInput(uint message, uint64_t wideParam, uint32_t param);
        virtual void Paint() {};
        virtual void OnClick() {};

    private:
        static LRESULT WindowProc(HWND, uint, uint64_t, int64_t);

        HWND _hwnd;

        const IWindow* _parent;
        std::string _text;
        u16vec2 _size;
    };

    template<typename FUNC_T> requires requires(FUNC_T f, IWindow& w) { { f(w) }; }
    class Button final : public IWindow
    {
    public:
        Button(const std::string& name, uvec2 pos, uvec2 size, FUNC_T&& func, const IWindow& window) :
            IWindow(IWindowType::Button, name, pos, size, &window),
            _func(std::move(func))
        {}

        Button(const std::string& name, uvec2 pos, uvec2 size, const FUNC_T& func, const IWindow& window) :
            IWindow(IWindowType::Button, name, pos, size, &window),
            _func(func)
        {}

    protected:
        void OnClick() override { _func(*this); }

    private:
        FUNC_T _func;
    };

    class Window final : public IWindow
    {
    public:
        explicit Window(uvec2 size) : IWindow(IWindowType::Window, "Discord", uvec2(0x80000000), size),
            _button("Test", uvec2(), uvec2(128, 128), [](const IWindow& w)
            {
                auto size = w.GetWindowSize();
                std::println("Window Size: ({}, {})", size.x, size.y);
            }, *this)
        {
        }

    private:
        Button<std::function<void(IWindow&)>> _button;
    };
}
