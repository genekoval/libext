#include <ext/detail/coroutine/mutex.hpp>

#include <utility>

namespace ext {
    auto mutex::lock() noexcept -> awaiter {
        return awaiter(*this);
    }

    auto mutex::unlock() noexcept -> void {
        if (awaiters.empty()) locked = false;
        else awaiters.pop();
    }

    auto mutex::queue_size() const noexcept -> std::size_t {
        return awaiters.size();
    }

    auto mutex::unlocked() const noexcept -> bool {
        return !locked;
    }

    mutex::awaiter::awaiter(mutex& mut) : mut(mut) {}

    auto mutex::awaiter::await_ready() const noexcept -> bool {
        if (mut.locked) return false;
        return mut.locked = true;
    }

    auto mutex::awaiter::await_suspend(
        std::coroutine_handle<> coroutine
    ) -> void {
        this->coroutine = coroutine;
        mut.awaiters.enqueue(*this);
    }

    auto mutex::awaiter::await_resume() -> guard {
        return guard(mut);
    }

    mutex::guard::guard(mutex& mut) : mut(&mut) {}

    mutex::guard::guard(guard&& other) :
        mut(std::exchange(other.mut, nullptr))
    {}

    mutex::guard::~guard() {
        unlock();
    }

    auto mutex::guard::operator=(guard&& other) noexcept -> guard& {
        if (std::addressof(other) != this) {
            unlock();
            mut = std::exchange(other.mut, nullptr);
        }

        return *this;
    }

    auto mutex::guard::unlock() -> void {
        auto* const mut = std::exchange(this->mut, nullptr);
        if (mut) mut->unlock();
    }
}
