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
    struct IWindowType
    {
        std::string_view Class;
        long Flags;
    };

    struct IWindowClass
    {
        std::string_view Name;
    };

    class IWindow
    {
    public:
        IWindow(IWindowType windowStyle, std::string_view name, uvec2 position, uvec2 size, const IWindow* parent = nullptr);
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

        static void RegisterStyle(const IWindowClass& windowClass);

        [[nodiscard]] HWND GetHWND() const { return _hwnd; }

    private:
        static LRESULT WindowProc(HWND, uint, uint64_t, int64_t);

        HWND _hwnd;

        const IWindow* _parent;
        u16vec2 _size;
    };

    template<class FUNC_T>
    concept IsButtonFunction = requires(FUNC_T f, IWindow& b) { { f(b) }; };

    template<typename FUNC_T> requires IsButtonFunction<FUNC_T>
    class IButton final : public IWindow
    {
    public:
        template<typename... ARGS>
        IButton(std::string_view text, uvec2 pos, uvec2 size, const IWindow& window, ARGS&&... args) :
            IWindow({ "Button", 0 }, text, pos, size, &window),
            _func(std::forward<ARGS>(args)...)
        {}

    protected:
        void OnCommand() override { _func(*this); }

    private:
        FUNC_T _func;
    };

    template<typename DELEGATE_T, typename FUNC_T>
    concept IsDelegateFunction = requires(FUNC_T f, DELEGATE_T* d, IWindow& b) { { f(d, b) }; };

    template<typename DELEGATE_T, typename FUNC_T = std::function<void(DELEGATE_T* d, IWindow& w)>> requires IsDelegateFunction<DELEGATE_T, FUNC_T>
    struct Delegate
    {
        DELEGATE_T* T;
        FUNC_T Function;

        void operator()(IWindow& window) { Function(T, window); }
    };

    using Button = IButton<std::function<void(IWindow&)>>;

    template<typename DELEGATE_T, typename FUNC_T = std::function<void(DELEGATE_T* d, IWindow& w)>>
    using DelegateButton = IButton<Delegate<DELEGATE_T, FUNC_T>>;

    class TextField final : public IWindow
    {
    public:
        TextField(uvec2 pos, uvec2 size, const IWindow& window, std::string_view defaultText = "");

        [[nodiscard]] const std::string& GetText() const { return _text; }
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
        Label(std::string_view text, uvec2 pos, uvec2 size, const IWindow& window);

        void SetText(std::string_view text) const;

    protected:
        void OnCommand() override {};
    };

    class Window : public IWindow
    {
    public:
        Window(std::string_view name, uvec2 position, uvec2 size);

        void RepaintAll() const;

    protected:
        void Paint() override;
        void OnCommand() override {};
    };
}