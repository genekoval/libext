#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <variant>

namespace ext {
    struct broken_promise : std::logic_error {
        broken_promise() : std::logic_error("broken promise") {}
    };

    struct no_result_error : std::exception {
        [[nodiscard]]
        auto what() const noexcept -> const char* override {
            return "result is not set";
        }
    };

    template <typename T>
    struct task;

    namespace detail {
        template <typename T>
        class result_container {
            std::variant<std::monostate, T, std::exception_ptr> value;

            constexpr auto res() -> T* {
                if (auto ex = std::get_if<std::exception_ptr>(&value)) {
                    std::rethrow_exception(*ex);
                }

                if (auto val = std::get_if<T>(&value)) {
                    return val;
                }

                throw no_result_error();
            }
        public:
            constexpr auto has_value() const noexcept -> bool {
                return std::get_if<std::monostate>(&value) == nullptr;
            }

            template <typename R>
            constexpr auto return_value(R&& value) noexcept -> void {
                return set_value(std::forward<R>(value));
            }

            template <typename R>
            constexpr auto set_value(R&& value) noexcept -> void {
                this->value.template emplace<T>(std::forward<R>(value));
            }

            constexpr auto result() & -> T {
                return *res();
            }

            constexpr auto result() && -> T {
                return std::move(*res());
            }

            auto set_exception(std::exception_ptr ex) noexcept -> void {
                value = ex;
            }

            auto unhandled_exception() noexcept -> void {
                value = std::current_exception();
            }
        };

        template <>
        class result_container<void> {
            std::optional<std::exception_ptr> exception;
        public:
            constexpr auto has_value() const noexcept -> bool {
                return exception.has_value();
            }

            constexpr auto return_void() noexcept -> void {
                exception.emplace(nullptr);
            }

            auto result() -> void {
                if (exception && *exception != nullptr) {
                    std::rethrow_exception(*exception);
                }
            }

            auto set_exception(std::exception_ptr ex) noexcept -> void {
                exception = ex;
            }

            auto unhandled_exception() noexcept -> void {
                exception = std::current_exception();
            }
        };

        class promise_base {
            std::coroutine_handle<> continuation;
        public:
            struct final_awaitable {
                auto await_ready() const noexcept -> bool {
                    return false;
                }

                template <typename Promise>
                auto await_suspend(
                    std::coroutine_handle<Promise> coroutine
                ) const noexcept -> std::coroutine_handle<> {
                    return coroutine.promise().continuation;
                }

                auto await_resume() const noexcept -> void {}
            };

            auto initial_suspend() const noexcept {
                return std::suspend_always();
            }

            auto final_suspend() noexcept {
                return final_awaitable();
            }

            auto set_continuation(
                std::coroutine_handle<> continuation
            ) noexcept -> void {
                this->continuation = continuation;
            }
        };
    }

    template <typename R = void>
    struct [[nodiscard("coroutine immediately destroyed")]] task {
        struct promise_type;
    private:
        using coroutine_handle = std::coroutine_handle<promise_type>;

        struct awaitable_base {
            coroutine_handle coroutine;

            awaitable_base(coroutine_handle coroutine) noexcept :
                coroutine(coroutine)
            {}

            auto await_ready() const noexcept -> bool {
                return !coroutine || coroutine.done();
            }

            auto await_suspend(
                std::coroutine_handle<> awaiting_coroutine
            ) noexcept -> std::coroutine_handle<> {
                coroutine.promise().set_continuation(awaiting_coroutine);
                return coroutine;
            }
        };

        coroutine_handle coroutine;
    public:
        struct promise_type final :
            detail::promise_base,
            detail::result_container<R>
        {
            auto get_return_object() noexcept -> task {
                return task(coroutine_handle::from_promise(*this));
            }
        };

        task() noexcept : coroutine(nullptr) {}

        explicit task(coroutine_handle coroutine) : coroutine(coroutine) {}

        task(const task&) = delete;

        task(task&& other) noexcept : coroutine(other.coroutine) {
            other.coroutine = nullptr;
        }

        ~task() {
            if (coroutine) coroutine.destroy();
        }

        auto operator=(const task&) -> task& = delete;

        auto operator=(task&& other) noexcept -> task& {
            if (std::addressof(other) != this) {
                if (coroutine) {
                    coroutine.destroy();
                }

                coroutine = other.coroutine;
                other.coroutine = nullptr;
            }

            return *this;
        }

        auto operator co_await() const & noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    if (!this->coroutine) {
                        throw broken_promise();
                    }

                    return this->coroutine.promise().result();
                }
            };

            return awaitable(coroutine);
        }

        auto operator co_await() const && noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    if (!this->coroutine) {
                        throw broken_promise();
                    }

                    return std::move(this->coroutine.promise()).result();
                }
            };

            return awaitable(coroutine);
        }

        /**
         * Query if the task result is complete.
         * Awaiting a task that is ready is guarenteed not to block/suspend.
         */
        auto is_ready() const noexcept -> bool {
            return !coroutine || coroutine.done();
        }

        /**
         * Returns an awaitable that will await completion of the task without
         * attempting to retrieve the result.
         */
        auto when_ready() const noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                auto await_resume() const noexcept {}
            };

            return awaitable(coroutine);
        }
    };

    struct detached_task {
        struct promise_type {
            auto get_return_object() -> detached_task;

            auto initial_suspend() -> std::suspend_never;

            auto final_suspend() noexcept -> std::suspend_never;

            auto return_void() noexcept -> void;

            auto unhandled_exception() -> void;
        };
    };

    auto make_task() -> ext::task<>;

    template <typename T>
    auto make_task(T t) -> ext::task<T> {
        co_return t;
    }
}
