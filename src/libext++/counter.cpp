#include <ext/detail/coroutine/counter.hpp>

#include <utility>

namespace ext {
    counter::operator bool() const noexcept { return value > 0; }

    auto counter::await(unsigned long threshold) -> awaitable {
        this->threshold = threshold;
        return awaitable(*this);
    }

    auto counter::count() const noexcept -> std::size_t { return value; }

    auto counter::decrement() noexcept -> void {
        if (--value <= threshold && coroutine && !coroutine.done())
            coroutine.resume();
    }

    auto counter::increment() noexcept -> guard {
        ++value;
        return guard(this);
    }

    counter::awaitable::awaitable(ext::counter& counter) : counter(counter) {}

    auto counter::awaitable::await_ready() const noexcept -> bool {
        return counter.value <= counter.threshold;
    }

    auto counter::awaitable::await_suspend(std::coroutine_handle<> coroutine
    ) noexcept -> void {
        counter.coroutine = coroutine;
    }

    auto counter::awaitable::await_resume() const noexcept -> void {
        counter.coroutine = nullptr;
    }

    counter::guard::guard(counter* origin) : origin(origin) {}

    counter::guard::guard(guard&& other) :
        origin(std::exchange(other.origin, nullptr)) {}

    counter::guard::~guard() {
        if (origin) origin->decrement();
    }

    auto counter::guard::operator=(guard&& other) -> guard& {
        if (std::addressof(other) != this) {
            if (origin) origin->decrement();
            origin = std::exchange(other.origin, nullptr);
        }

        return *this;
    }
}
