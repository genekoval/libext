#pragma once

#include <concepts>
#include <coroutine>
#include <exception>
#include <iterator>
#include <variant>

namespace ext {
    template <typename T>
    struct [[nodiscard]] generator {
        struct promise_type;
    private:
        using coroutine_handle = std::coroutine_handle<promise_type>;

        coroutine_handle coroutine;
        bool full = false;

        auto fill() -> void {
            if (full) return;

            coroutine.resume();

            const auto& value = coroutine.promise().value;
            if (const auto* ex = std::get_if<std::exception_ptr>(&value)) {
                std::rethrow_exception(*ex);
            }

            full = true;
        }
    public:
        struct promise_type {
            std::variant<std::monostate, T, std::exception_ptr> value;

            auto get_return_object() noexcept -> generator {
                return generator(coroutine_handle::from_promise(*this));
            }

            auto initial_suspend() noexcept -> std::suspend_always {
                return {};
            }

            auto final_suspend() noexcept -> std::suspend_always { return {}; }

            auto unhandled_exception() -> void {
                value = std::current_exception();
            }

            template <std::convertible_to<T> From>
            auto yield_value(From&& from) -> std::suspend_always {
                value = std::forward<From>(from);
                return {};
            }

            auto return_void() -> void {}
        };

        generator() noexcept : coroutine(nullptr) {}

        explicit generator(coroutine_handle coroutine) : coroutine(coroutine) {}

        generator(const generator&) = delete;

        generator(generator&& other) noexcept : coroutine(other.coroutine) {
            other.coroutine = nullptr;
        }

        ~generator() {
            if (coroutine) coroutine.destroy();
        }

        auto operator=(const generator&) -> generator& = delete;

        auto operator=(generator&& other) noexcept -> generator& {
            if (std::addressof(other) != this) {
                if (coroutine) { coroutine.destroy(); }

                coroutine = other.coroutine;
                other.coroutine = nullptr;

                full = other.full;
            }

            return *this;
        }

        explicit operator bool() {
            fill();
            return !coroutine.done();
        }

        auto operator()() -> T {
            fill();
            full = false;

            const auto& value = coroutine.promise().value;
            return std::move(std::get<T>(value));
        }
    };
}
