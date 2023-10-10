#include <ext/detail/coroutine/awaiter_queue.hpp>

#include <utility>

namespace ext {
    awaiter_queue::awaiter_queue(awaiter_queue&& other) :
        head(std::exchange(other.head, nullptr)),
        tail(std::exchange(other.tail, nullptr)) {}

    auto awaiter_queue::operator=(awaiter_queue&& other) noexcept
        -> awaiter_queue& {
        if (std::addressof(other) != this) {
            head = std::exchange(other.head, nullptr);
            tail = std::exchange(other.tail, nullptr);
        }

        return *this;
    }

    auto awaiter_queue::clear() noexcept -> void {
        head = nullptr;
        tail = nullptr;
    }

    auto awaiter_queue::empty() const noexcept -> bool {
        return head == nullptr;
    }

    auto awaiter_queue::enqueue(awaiter_node& awaiter) noexcept -> void {
        if (!head) head = &awaiter;
        if (tail) tail->next = &awaiter;
        tail = &awaiter;
    }

    auto awaiter_queue::enqueue(awaiter_queue& other) noexcept -> void {
        if (other.empty()) return;

        enqueue(*other.head);

        tail = other.tail;

        other.clear();
    }

    auto awaiter_queue::resume() -> void {
        // Make a local copy of the awaiter list, and create a new list
        // since resumed coroutine could add more awaiters.
        auto* current = head;
        clear();

        while (current) {
            // After we resume a coroutine,
            // the object pointed to by `current` may cease to exist.
            const auto coroutine = current->coroutine;
            current = current->next;

            if (coroutine && !coroutine.done()) coroutine.resume();
        }
    }

    auto awaiter_queue::pop() -> void {
        if (!head) return;

        auto& awaiter = *head;

        if (head == tail) clear();
        else head = std::exchange(awaiter.next, nullptr);

        const auto coroutine = awaiter.coroutine;
        if (coroutine && !coroutine.done()) coroutine.resume();
    }

    auto awaiter_queue::size() const noexcept -> std::size_t {
        std::size_t n = 0;

        auto* current = head;

        while (current) {
            ++n;
            current = current->next;
        }

        return n;
    }
}
