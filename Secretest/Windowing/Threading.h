//
// Created by scion on 1/14/2026.
//

#pragma once

#include <functional>
#include <list>
#include <utility>
#include <memory>
#include <mutex>
#include <print>

namespace Secretest
{
    struct ITask
    {
        virtual void operator()() = 0;
        virtual ~ITask() = default;
    };

    template<typename FUNC_T, typename... ARGS>
    concept IsTask = requires(FUNC_T& f, std::tuple<ARGS...>& a) { { std::apply(f, a) }; };

    template<typename FUNC_T, typename... ARGS> requires IsTask<FUNC_T, ARGS...>
    struct Task : ITask
    {
        explicit Task(FUNC_T func, ARGS... args) : Function(func), Args(std::forward<ARGS>(args)...) {}

        FUNC_T Function;
        std::tuple<ARGS...> Args;

        void operator()() override { std::apply(Function, Args); }
    };

    template<typename... ARGS>
    using FTask = Task<std::function<void(ARGS...)>, ARGS...>;

    class TaskQueue
    {
    public:
        TaskQueue() = default;

        template<typename TASK_T>
        void Push(TASK_T&& task)
        {
            std::unique_lock lock(_mutex);
            _queue.push_back(std::forward<TASK_T>(task));
        }

        template<typename FUNC_T, typename... ARGS>
        void Emplace(FUNC_T&& func, ARGS&&... args)
        {
            std::unique_lock lock(_mutex);
            _queue.push_back(std::make_unique<Task<FUNC_T, ARGS...>>(std::forward<FUNC_T>(func), std::forward<ARGS>(args)...));
        }

        void RunAllTasks()
        {
            std::unique_lock lock(_mutex);

            for(const auto& item : _queue)
                item->operator()();

            _queue.clear();
        }

        [[nodiscard]] bool HasTasks()
        {
            std::unique_lock lock(_mutex);
            return !_queue.empty();
        }

    private:
        std::mutex _mutex;
        std::list<std::unique_ptr<ITask>> _queue;
    };
}
