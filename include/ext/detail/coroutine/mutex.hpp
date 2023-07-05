#pragma once

#include <coroutine>
#include <queue>

namespace ext {
    class mutex {
        std::queue<std::coroutine_handle<>> coroutines;
        bool ready = true;

        auto unlock() noexcept -> void;
    public:
        friend class guard;

        class awaiter;

        class guard {
            friend class awaiter;

            mutex& mut;

            guard(mutex& mut);
        public:
            ~guard();
        };

        class awaiter {
            friend class mutex;

            mutex& mut;

            awaiter(mutex& mut);
        public:
            auto await_ready() const noexcept -> bool;

            auto await_suspend(std::coroutine_handle<> coroutine) -> void;

            auto await_resume() -> guard;
        };

        mutex() = default;

        mutex(const mutex&) = delete;

        mutex(mutex&&) = delete;

        auto operator=(const mutex&) -> mutex& = delete;

        auto operator=(mutex&&) -> mutex& = delete;

        auto lock() noexcept -> awaiter;

        auto locked() const noexcept -> bool;

        auto queue_size() const noexcept -> std::size_t;
    };
}
