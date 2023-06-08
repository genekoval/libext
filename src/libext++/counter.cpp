#include <ext/detail/coroutine/counter.hpp>

#include <utility>

namespace ext {
    counter::operator bool() const noexcept {
        return value > 0;
    }

    auto counter::await_ready() const noexcept -> bool {
        return value == 0;
    }

    auto counter::await_suspend(
        std::coroutine_handle<> coroutine
    ) noexcept -> void {
        this->coroutine = coroutine;
    }

    auto counter::await_resume() const noexcept -> void {}

    auto counter::count() const noexcept -> std::size_t {
        return value;
    }

    auto counter::decrement() noexcept -> void {
        if (--value == 0 && coroutine && !coroutine.done()) coroutine.resume();
    }

    auto counter::increment() noexcept -> counter_item {
        ++value;
        return counter_item(this);
    }

    counter_item::counter_item(counter* origin) : origin(origin) {}

    counter_item::counter_item(counter_item&& other) :
        origin(std::exchange(other.origin, nullptr))
    {}

    counter_item::~counter_item() {
        if (origin) origin->decrement();
    }

    auto counter_item::operator=(counter_item&& other) -> counter_item& {
        if (std::addressof(other) != this) {
            if (origin) origin->decrement();
            origin = std::exchange(other.origin, nullptr);
        }

        return *this;
    }
}
