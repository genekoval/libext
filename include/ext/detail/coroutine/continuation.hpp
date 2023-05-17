#pragma once

#include "except.hpp"

#include <coroutine>
#include <exception>
#include <utility>
#include <variant>

namespace ext {
    template <typename T = void>
    class continuation final {
        friend struct awaitable_base;

        class awaitable_base {
            continuation* cont;
        protected:
            auto res() -> T* {
                if (auto* val = std::get_if<T>(&cont->value)) return val;

                if (auto* ex = std::get_if<std::exception_ptr>(&cont->value)) {
                    std::rethrow_exception(*ex);
                }

                throw no_result();
            }
        public:
            awaitable_base(continuation* cont) : cont(cont) {}

            auto await_ready() const noexcept -> bool {
                return false;
            }

            auto await_suspend(std::coroutine_handle<> coroutine) -> void {
                cont->coroutine = coroutine;
            }
        };

        std::coroutine_handle<> coroutine;
        std::variant<std::monostate, T, std::exception_ptr> value;

        auto resume() -> void {
            std::exchange(coroutine, nullptr).resume();
        }
    public:
        continuation() = default;

        continuation(const continuation&) = delete;

        continuation(continuation&&) = default;

        auto operator=(const continuation&) -> continuation& = delete;

        auto operator=(continuation&&) -> continuation& = default;

        explicit operator bool() const noexcept {
            return awaiting();
        }

        auto operator co_await() & noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    return *this->res();
                }
            };

            return awaitable(this);
        }

        auto operator co_await() && noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    return std::move(*this->get());
                }
            };

            return awaitable(this);
        }

        auto awaiting() const noexcept -> bool{
            return coroutine != nullptr;
        }

        auto resume(T&& t) -> void {
            value.emplace(std::forward<T>(t));
            resume();
        }

        auto resume(std::exception_ptr exception) -> void {
            value.emplace(exception);
            resume();
        }
    };

    template <>
    class continuation<void> final {
        std::coroutine_handle<> coroutine;
        std::exception_ptr exception;

        class awaitable {
            continuation* cont;
        public:
            awaitable(continuation* cont) : cont(cont) {}

            auto await_ready() const noexcept -> bool {
                return false;
            }

            auto await_suspend(std::coroutine_handle<> coroutine) -> void {
                cont->coroutine = coroutine;
            }

            auto await_resume() -> void {
                if (cont->exception) std::rethrow_exception(cont->exception);
            }
        };
    public:
        continuation() = default;

        continuation(const continuation&) = delete;

        continuation(continuation&&) = default;

        auto operator=(const continuation&) -> continuation& = delete;

        auto operator=(continuation&&) -> continuation& = default;

        explicit operator bool() const noexcept {
            return awaiting();
        }

        auto operator co_await() noexcept {
            return awaitable(this);
        }

        auto awaiting() const noexcept -> bool{
            return coroutine != nullptr;
        }

        auto resume() -> void {
            std::exchange(coroutine, nullptr).resume();
        }

        auto resume(std::exception_ptr exception) -> void {
            this->exception = exception;
            resume();
        }
    };
}
