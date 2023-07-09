#pragma once

#include <coroutine>

namespace ext {
    class counter final {
        friend class guard;

        std::coroutine_handle<> coroutine;
        unsigned long value = 0;
        unsigned long threshold = 0;

        auto decrement() noexcept -> void;
    public:
        class awaitable {
            friend class ext::counter;

            ext::counter& counter;

            awaitable(ext::counter& counter);
        public:
            auto await_ready() const noexcept -> bool;

            auto await_suspend(
                std::coroutine_handle<> coroutine
            ) noexcept -> void;

            auto await_resume() const noexcept -> void;
        };

        class guard {
            friend class counter;

            counter* origin = nullptr;

            guard(counter* origin);
        public:
            guard() = default;

            guard(const guard&) = delete;

            guard(guard&& other);

            ~guard();

            auto operator=(const guard&) -> guard& = delete;

            auto operator=(guard&& other) -> guard&;
        };

        counter() = default;

        counter(const counter&) = delete;

        counter(counter&&) = delete;

        auto operator=(const counter&) -> counter& = delete;

        auto operator=(counter&&) -> counter& = delete;

        explicit operator bool() const noexcept;

        auto count() const noexcept -> unsigned long;

        [[nodiscard("counter immediately decremented")]]
        auto increment() noexcept -> guard;

        auto await(unsigned long threshold = 0) -> awaitable;
    };
}
