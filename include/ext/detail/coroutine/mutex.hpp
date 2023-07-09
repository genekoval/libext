#pragma once

#include "awaiter_queue.hpp"

namespace ext {
    class mutex {
        awaiter_queue awaiters;
        bool locked = false;

        auto unlock() noexcept -> void;
    public:
        friend class guard;

        class awaiter;

        class guard {
            friend class awaiter;

            mutex* mut = nullptr;

            guard(mutex& mut);
        public:
            guard() = default;

            guard(const guard&) = delete;

            guard(guard&& other);

            ~guard();

            auto operator=(const guard&) -> guard& = delete;

            auto operator=(guard&& other) noexcept -> guard&;

            auto unlock() -> void;
        };

        class awaiter : awaiter_node {
            friend class mutex;

            mutex& mut;

            awaiter(mutex& mut);
        public:
            auto await_ready() const noexcept -> bool;

            auto await_suspend(std::coroutine_handle<> coroutine) -> void;

            [[nodiscard("mutex immediately unlocked")]]
            auto await_resume() -> guard;
        };

        mutex() = default;

        mutex(const mutex&) = delete;

        mutex(mutex&&) = delete;

        auto operator=(const mutex&) -> mutex& = delete;

        auto operator=(mutex&&) -> mutex& = delete;

        [[nodiscard("result must be awaited")]]
        auto lock() noexcept -> awaiter;

        auto queue_size() const noexcept -> std::size_t;

        auto unlocked() const noexcept -> bool;
    };
}
