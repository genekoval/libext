#include <ext/detail/coroutine/task.hpp>

namespace ext {
    auto detached_task::promise_type::get_return_object() -> detached_task {
        return {};
    }

    auto detached_task::promise_type::initial_suspend() -> std::suspend_never {
        return {};
    }

    auto detached_task::promise_type::final_suspend() noexcept
    -> std::suspend_never {
        return {};
    }

    auto detached_task::promise_type::return_void() noexcept -> void {}

    auto detached_task::promise_type::unhandled_exception() -> void {
        std::terminate();
    }

    auto make_task() -> ext::task<> {
        co_return;
    }
}
