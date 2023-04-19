#pragma once

#include "except.hpp"

#include <variant>

namespace ext::detail {
    template <typename T>
    class result {
        std::variant<std::monostate, T, std::exception_ptr> value;

        auto res() -> T* {
            if (auto* val = std::get_if<T>(&value)) return val;

            if (auto* ex = std::get_if<std::exception_ptr>(&value)) {
                std::rethrow_exception(*ex);
            }

            throw no_result();
        }
    public:
        auto get() & -> T& {
            return *res();
        }

        auto get() && -> T {
            return std::move(*res());
        }

        template <typename R>
        auto return_value(R&& value) noexcept -> void {
            this->value.template emplace<T>(std::forward<R>(value));
        }

        auto unhandled_exception() noexcept -> void {
            value = std::current_exception();
        }
    };

    template <>
    class result<void> {
        bool complete = false;
        std::exception_ptr exception;
    public:
        auto get() const -> void {
            if (exception) std::rethrow_exception(exception);
            if (!complete) throw no_result();
        }

        auto return_void() noexcept -> void {
            complete = true;
        }

        auto unhandled_exception() noexcept -> void {
            exception = std::current_exception();
        }
    };
}
