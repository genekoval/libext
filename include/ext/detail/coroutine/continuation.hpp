#pragma once

#include "except.hpp"

#include <coroutine>
#include <exception>
#include <utility>
#include <variant>

namespace ext {
    template <typename T = void>
    class continuation final {
        friend struct awaitable;

        class awaitable {
            continuation* cont;
        public:
            awaitable(continuation* cont) : cont(cont) {}

            ~awaitable() {
                if (cont->coroutine) cont->coroutine = nullptr;
            }

            auto await_ready() const noexcept -> bool {
                return !std::holds_alternative<std::monostate>(cont->value);
            }

            auto await_suspend(std::coroutine_handle<> coroutine) -> void {
                cont->coroutine = coroutine;
            }

            auto await_resume() -> T {
                if (auto* val = std::get_if<T>(&cont->value)) {
                    auto result = std::move(*val);
                    cont->value = std::monostate();
                    return result;
                }

                if (auto* ex = std::get_if<std::exception_ptr>(&cont->value)) {
                    auto result = std::move(*ex);
                    cont->value = std::monostate();
                    std::rethrow_exception(result);
                }

                throw no_result();
            }
        };

        std::coroutine_handle<> coroutine;
        std::variant<std::monostate, T, std::exception_ptr> value;

        auto resume() -> void {
            if (coroutine) std::exchange(coroutine, nullptr).resume();
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

        auto operator co_await() noexcept {
            return awaitable(this);
        }

        auto awaiting() const noexcept -> bool {
            return coroutine != nullptr;
        }

        template <std::convertible_to<T> U>
        auto resume(U&& u) -> void {
            value.template emplace<T>(std::forward<U>(u));
            resume();
        }

        auto resume(std::exception_ptr exception) -> void {
            value = exception;
            resume();
        }
    };

    template <>
    class continuation<void> final {
        std::coroutine_handle<> coroutine;
        std::exception_ptr exception;
        bool ready = false;

        class awaitable final {
            std::coroutine_handle<> coroutine;
            continuation* cont;
        public:
            awaitable(continuation* cont) : cont(cont) {}

            awaitable(const awaitable&) = delete;

            awaitable(awaitable&& other) :
                coroutine(std::exchange(other.coroutine, nullptr)),
                cont(other.cont)
            {}

            ~awaitable() {
                if (cont->coroutine == coroutine) {
                    cont->coroutine = nullptr;
                }
            }

            auto operator=(const awaitable&) -> awaitable& = delete;

            auto operator=(awaitable&& other) -> awaitable& {
                if (std::addressof(other) != this) {
                    std::destroy_at(this);
                    std::construct_at(this, std::forward<awaitable>(other));
                }

                return *this;
            }

            auto await_ready() const noexcept -> bool {
                return cont->ready;
            }

            auto await_suspend(
                std::coroutine_handle<> coroutine
            ) -> std::coroutine_handle<> {
                this->coroutine = coroutine;
                auto tmp = std::exchange(cont->coroutine, coroutine);
                return tmp ? tmp : std::noop_coroutine();
            }

            auto await_resume() -> void {
                cont->ready = false;

                if (cont->exception) {
                    std::rethrow_exception(std::exchange(
                        cont->exception,
                        nullptr
                    ));
                }
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
            if (coroutine) std::exchange(coroutine, nullptr).resume();
            else ready = true;
        }

        auto resume(std::exception_ptr exception) -> void {
            this->exception = exception;
            resume();
        }
    };
}
