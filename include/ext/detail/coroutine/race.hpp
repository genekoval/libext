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
                        return coroutine.promise().continuation;
                    }

                    auto await_resume() const noexcept -> void {}
                };

                std::coroutine_handle<> continuation;

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
            ) const noexcept -> void {
                coroutine.promise().continuation = continuation;
                coroutine.resume();
            }
        };

        bool done = false;
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
            done = true;
            result = result_type(std::in_place_index<I>, std::move(task));
        }

        auto start_tasks(std::coroutine_handle<> coroutine) -> void {
            for (const auto& task : tasks) {
                task.start(coroutine);
                if (done) return;
            }
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
            start_tasks(coroutine);
        }

        auto await_resume() -> result_type {
            return std::move(result);
        }
    };
}
