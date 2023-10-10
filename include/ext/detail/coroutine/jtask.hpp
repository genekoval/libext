#pragma once

#include "result.hpp"

#include <coroutine>
#include <utility>

namespace ext {
    template <typename R = void>
    struct [[nodiscard("coroutine immediately destroyed")]] jtask final {
        using value_type = R;

        struct promise_type;
    private:
        using coroutine_handle = std::coroutine_handle<promise_type>;

        struct awaitable_base {
            coroutine_handle coroutine;

            awaitable_base(coroutine_handle coroutine) noexcept :
                coroutine(coroutine) {}

            auto await_ready() const noexcept -> bool {
                return !coroutine || coroutine.done();
            }

            auto await_suspend(std::coroutine_handle<> awaiting) noexcept
                -> void {
                coroutine.promise().continuation = awaiting;
            }
        };

        coroutine_handle coroutine;
    public:
        struct promise_type final : detail::result<R> {
            std::coroutine_handle<> continuation;

            struct final_awaitable {
                auto await_ready() const noexcept -> bool { return false; }

                template <typename Promise>
                auto await_suspend(std::coroutine_handle<Promise> coroutine
                ) const noexcept -> std::coroutine_handle<> {
                    const auto continuation = coroutine.promise().continuation;

                    if (continuation) return continuation;
                    return std::noop_coroutine();
                }

                auto await_resume() const noexcept -> void {}
            };

            auto get_return_object() noexcept -> jtask {
                return jtask(coroutine_handle::from_promise(*this));
            }

            auto initial_suspend() const noexcept {
                return std::suspend_never();
            }

            auto final_suspend() noexcept { return final_awaitable(); }
        };

        jtask() noexcept : coroutine(nullptr) {}

        explicit jtask(coroutine_handle coroutine) : coroutine(coroutine) {}

        jtask(const jtask&) = delete;

        jtask(jtask&& other) noexcept :
            coroutine(std::exchange(other.coroutine, nullptr)) {}

        ~jtask() {
            if (coroutine) coroutine.destroy();
        }

        auto operator=(const jtask&) -> jtask& = delete;

        auto operator=(jtask&& other) noexcept -> jtask& {
            if (std::addressof(other) != this) {
                if (coroutine) coroutine.destroy();
                coroutine = std::exchange(other.coroutine, nullptr);
            }

            return *this;
        }

        auto operator co_await() const& noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    if (!this->coroutine) throw broken_promise();
                    return this->coroutine.promise().get();
                }
            };

            return awaitable(coroutine);
        }

        auto operator co_await() const&& noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    if (!this->coroutine) throw broken_promise();
                    return std::move(this->coroutine.promise()).get();
                }
            };

            return awaitable(coroutine);
        }

        /**
         * Query if the task result is complete.
         * Awaiting a task that is ready is guarenteed not to block/suspend.
         */
        auto is_ready() const noexcept -> bool {
            return !coroutine || coroutine.done();
        }

        auto joinable() const noexcept -> bool { return coroutine != nullptr; }

        auto result() const& {
            if (!coroutine) throw broken_promise();
            return coroutine.promise().get();
        }

        auto result() const&& {
            if (!coroutine) throw broken_promise();
            return std::move(coroutine.promise()).get();
        }

        /**
         * Returns an awaitable that will await completion of the task without
         * attempting to retrieve the result.
         */
        auto when_ready() const noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                auto await_resume() const noexcept {}
            };

            return awaitable(coroutine);
        }
    };

}
