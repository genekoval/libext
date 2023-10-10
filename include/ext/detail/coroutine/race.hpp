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
                    auto await_ready() const noexcept -> bool { return false; }

                    template <typename Promise>
                    auto await_suspend(std::coroutine_handle<Promise> coroutine
                    ) const noexcept -> std::coroutine_handle<> {
                        const auto continuation =
                            coroutine.promise().context.coroutine;

                        if (continuation) return continuation;
                        return std::noop_coroutine();
                    }

                    auto await_resume() const noexcept -> void {}
                };

                race& context;

                promise_type(race& context, auto&) : context(context) {}

                auto get_return_object() noexcept -> task {
                    return task(coroutine_handle::from_promise(*this));
                }

                auto initial_suspend() const noexcept {
                    return std::suspend_always();
                }

                auto final_suspend() noexcept { return final_awaitable(); }
            };

            task() = default;

            explicit task(coroutine_handle coroutine) : coroutine(coroutine) {}

            task(const task&) = delete;

            task(task&& other) noexcept :
                coroutine(std::exchange(other.coroutine, nullptr)) {}

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

            auto done() const noexcept -> bool { return coroutine.done(); }

            auto resume() const -> void { coroutine.resume(); }
        };

        std::coroutine_handle<> coroutine;
        result_type result;
        std::array<task, sizeof...(Tasks)> tasks;

        template <std::size_t... I>
        auto make_tasks(task_tuple&& storage, std::index_sequence<I...>)
            -> void {
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

        race(const race&) = delete;

        race(race&&) = delete;

        auto operator=(const race&) -> race& = delete;

        auto operator=(race&&) -> race& = delete;

        auto await_ready() noexcept -> bool {
            for (const auto& task : tasks) {
                task.resume();
                if (task.done()) return true;
            }

            return false;
        }

        auto await_suspend(std::coroutine_handle<> coroutine) -> void {
            this->coroutine = coroutine;
        }

        auto await_resume() -> result_type { return std::move(result); }
    };
}
