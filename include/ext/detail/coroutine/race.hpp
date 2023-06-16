#pragma once

#include "result.hpp"

#include <coroutine>
#include <tuple>
#include <utility>

namespace ext {
    template <typename... Tasks>
    class race {
        using task_tuple = std::tuple<Tasks...>;
        using result_type = std::variant<Tasks...>;

        struct task {
            struct promise_type;
        private:
            using coroutine_handle = std::coroutine_handle<promise_type>;

            coroutine_handle coroutine;
        public:
            struct promise_type final : detail::result<void> {
                struct final_awaitable {
                    auto await_ready() const noexcept -> bool {
                        return false;
                    }

                    template <typename Promise>
                    auto await_suspend(
                        std::coroutine_handle<Promise> coroutine
                    ) const noexcept -> std::coroutine_handle<> {
                        auto& promise = coroutine.promise();

                        // If the `done` pointer is not null, the race object
                        // is still in the process of starting up.
                        if (auto* done = promise.done) *done = true;

                        return promise.continuation;
                    }

                    auto await_resume() const noexcept -> void {}
                };

                std::coroutine_handle<> continuation;
                bool* done = nullptr;

                auto get_return_object() noexcept -> task {
                    return task(coroutine_handle::from_promise(*this));
                }

                auto initial_suspend() const noexcept {
                    return std::suspend_always();
                }

                auto final_suspend() noexcept {
                    return final_awaitable();
                }
            };

            task() = default;

            explicit task(coroutine_handle coroutine) : coroutine(coroutine) {}

            task(const task&) = delete;

            task(task&& other) noexcept :
                coroutine(std::exchange(other.coroutine, nullptr))
            {}

            ~task() {
                if (coroutine) coroutine.destroy();
            }

            auto operator=(const task&) -> task& = delete;

            auto operator=(task&& other) noexcept -> task& {
                if (std::addressof(other) != this) {
                    if (coroutine) coroutine.destroy();
                    coroutine = std::exchange(other.coroutine, nullptr);
                }

                return *this;
            }

            auto start(
                std::coroutine_handle<> continuation
            ) const noexcept -> bool {
                auto done = false;
                auto& promise = coroutine.promise();

                promise.continuation = continuation;
                promise.done = &done;

                coroutine.resume();

                if (!done) promise.done = nullptr;
                return done;
            }
        };

        result_type result;
        std::array<task, sizeof...(Tasks)> tasks;

        template <std::size_t... I>
        auto make_tasks(
            task_tuple&& storage,
            std::index_sequence<I...>
        ) -> void {
            ((tasks[I] = run<I>(std::move(std::get<I>(storage)))), ...);
        }

        template <std::size_t I, typename T>
        auto run(T task) -> race::task {
            co_await task.when_ready();
            result = result_type(std::in_place_index<I>, std::move(task));
        }
    public:
        race(Tasks&&... tasks) {
            make_tasks(
                task_tuple(std::forward<Tasks>(tasks)...),
                std::index_sequence_for<Tasks...>()
            );
        }

        auto await_ready() noexcept -> bool {
            return false;
        }

        auto await_suspend(std::coroutine_handle<> coroutine) -> void {
            for (const auto& task : tasks) {
                if (task.start(coroutine)) return;
            }
        }

        auto await_resume() -> result_type {
            return std::move(result);
        }
    };
}
