#pragma once

#include <coroutine>

namespace ext {
    struct awaiter_node {
        std::coroutine_handle<> coroutine;
        awaiter_node* next = nullptr;
    };

    class awaiter_queue final {
        awaiter_node* head = nullptr;
        awaiter_node* tail = nullptr;
    public:
        awaiter_queue() = default;

        awaiter_queue(const awaiter_queue&) = delete;

        awaiter_queue(awaiter_queue&& other);

        auto operator=(const awaiter_queue&) -> awaiter_queue& = delete;

        auto operator=(awaiter_queue&& other) noexcept -> awaiter_queue&;

        auto clear() noexcept -> void;

        auto empty() const noexcept -> bool;

        auto enqueue(awaiter_node& awaiter) noexcept -> void;

        auto enqueue(awaiter_queue& other) noexcept -> void;

        auto resume() -> void;

        auto pop() -> void;

        auto size() const noexcept -> std::size_t;
    };
}
