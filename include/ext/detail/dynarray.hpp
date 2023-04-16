#pragma once

#include <cstring>
#include <memory>
#include <span>
#include <utility>

namespace ext {
    template <typename T, typename Allocator = std::allocator<T>>
    class dynarray final {
        Allocator allocator;
        std::size_t cap = 0;
        std::size_t items = 0;
        T* storage = nullptr;
    public:
        using value_type = T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer =
            typename std::allocator_traits<Allocator>::const_pointer;

        dynarray() noexcept(noexcept(Allocator())) = default;

        explicit dynarray(const Allocator& alloc) noexcept :
            allocator(alloc)
        {}

        explicit dynarray(
            size_type capacity,
            const Allocator& alloc = Allocator()
        ) :
            allocator(alloc),
            cap(capacity),
            storage(capacity == 0 ? nullptr : allocator.allocate(capacity))
        {}

        dynarray(
            std::initializer_list<T> init,
            const Allocator& alloc = Allocator()
        ) :
            dynarray(init.size(), alloc)
        {
            for (auto&& item : init) emplace_back(std::move(item));
        }

        dynarray(const dynarray&) = delete;

        dynarray(dynarray&& other) :
            allocator(other.allocator),
            cap(std::exchange(other.cap, 0)),
            items(std::exchange(other.items, 0)),
            storage(std::exchange(other.storage, nullptr))
        {}

        ~dynarray() {
            if (!storage) return;

            clear();
            allocator.deallocate(storage, cap);
        }

        auto operator=(const dynarray&) -> dynarray& = delete;

        auto operator=(dynarray&& other) -> dynarray& {
            std::destroy_at(this);
            std::construct_at(this, std::forward<dynarray>(other));
            return *this;
        }

        auto operator[](size_type pos) -> reference {
            return *(storage + pos);
        }

        auto operator[](size_type pos) const -> const_reference {
            return *(storage + pos);
        }

        auto get_allocator() const noexcept -> allocator_type {
            return allocator;
        }

        auto begin() noexcept -> pointer {
            return storage;
        }

        auto begin() const noexcept -> const_pointer {
            return storage;
        }

        auto cbegin() const noexcept -> const_pointer {
            return storage;
        }

        auto end() noexcept -> pointer {
            return storage + items;
        }

        auto end() const noexcept -> const_pointer {
            return storage + items;
        }

        auto cend() const noexcept -> const_pointer {
            return storage + items;
        }

        auto capacity() const noexcept -> size_type {
            return cap;
        }

        auto clear() -> void {
            std::destroy(begin(), end());
            items = 0;
        }

        auto copy(std::span<const T> src) -> void {
            std::memcpy(end(), src.data(), src.size());
            items += src.size();
        }

        auto data() noexcept -> T* {
            return storage;
        }

        auto data() const noexcept -> const T* {
            return storage;
        }

        template <typename... Args>
        auto emplace_back(Args&&... args) -> reference {
            T* const item = end();
            std::construct_at(item, std::forward<Args>(args)...);

            ++items;
            return *item;
        }

        auto empty() const noexcept -> bool {
            return items == 0;
        }

        auto front() noexcept -> reference {
            return *begin();
        }

        auto front() const noexcept -> const_reference {
            return *begin();
        }

        auto back() noexcept -> reference {
            return *std::prev(end());
        }

        auto back() const noexcept -> const_reference {
            return *std::prev(end());
        }

        auto size() const noexcept -> size_type {
            return items;
        }

        auto pop_back() -> void {
            --items;
            std::destroy_at(end());
        }
    };
}
