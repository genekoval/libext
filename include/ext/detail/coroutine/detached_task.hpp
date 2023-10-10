#pragma once

#include <coroutine>
#include <exception>

namespace ext {
    struct detached_task {
        struct promise_type {
            auto get_return_object() const noexcept { return detached_task(); }

            auto initial_suspend() const noexcept {
                return std::suspend_never();
            }

            auto final_suspend() const noexcept { return std::suspend_never(); }

            auto return_void() const noexcept -> void {}

            [[noreturn]]
            auto unhandled_exception() const noexcept -> void {
                std::terminate();
            }
        };
    };
}
