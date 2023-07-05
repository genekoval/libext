#include <ext/detail/coroutine/mutex.hpp>

namespace ext {
    auto mutex::lock() noexcept -> awaiter {
        return awaiter(*this);
    }

    auto mutex::locked() const noexcept -> bool {
        return !ready;
    }

    auto mutex::unlock() noexcept -> void {
        if (coroutines.empty()) {
            ready = true;
            return;
        }

        const auto next = coroutines.front();
        coroutines.pop();

        next.resume();
    }

    auto mutex::queue_size() const noexcept -> std::size_t {
        return coroutines.size();
    }

    mutex::awaiter::awaiter(mutex& mut) : mut(mut) {}

    auto mutex::awaiter::await_ready() const noexcept -> bool {
        if (!mut.ready) return false;

        mut.ready = false;
        return true;
    }

    auto mutex::awaiter::await_suspend(
        std::coroutine_handle<> coroutine
    ) -> void {
        mut.coroutines.emplace(coroutine);
    }

    auto mutex::awaiter::await_resume() -> guard {
        return guard(mut);
    }

    mutex::guard::guard(mutex& mut) : mut(mut) {}

    mutex::guard::~guard() {
        mut.unlock();
    }
}
