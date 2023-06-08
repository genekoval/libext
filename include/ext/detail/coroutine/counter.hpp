#pragma once

#include <coroutine>

namespace ext {
    class counter;

    class counter_item {
        friend class counter;

        counter* origin = nullptr;

        counter_item(counter* origin);
    public:
        counter_item() = default;

        counter_item(const counter_item&) = delete;

        counter_item(counter_item&& other);

        ~counter_item();

        auto operator=(const counter_item&) -> counter_item& = delete;

        auto operator=(counter_item&& other) -> counter_item&;
    };

    class counter final {
        friend class counter_item;

        std::coroutine_handle<> coroutine;
        std::size_t value = 0;

        auto decrement() noexcept -> void;
    public:
        counter() = default;

        counter(const counter&) = delete;

        counter(counter&&) = delete;

        auto operator=(const counter&) -> counter& = delete;

        auto operator=(counter&&) -> counter& = delete;

        explicit operator bool() const noexcept;

        auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> coroutine) noexcept -> void;

        auto await_resume() const noexcept -> void;

        auto count() const noexcept -> std::size_t;

        auto increment() noexcept -> counter_item;
    };
}
