#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

namespace ext {
    template <typename T>
    class pool final {
        std::vector<std::unique_ptr<T>> storage;
        std::vector<T*> available;

        auto expand(std::size_t size) -> void {
            storage.reserve(size);
            available.reserve(size);

            for (auto i = 0ul; i < size; ++i) {
                storage.emplace_back(config.create());
                available.push_back(storage.back().get());
            }
        }

        auto put_back(T* item) noexcept -> void {
            available.push_back(item);
        }
    public:
        using initializer = std::function<std::unique_ptr<T>()>;

        class item final {
            friend class pool;

            T* data;
            pool* origin;

            item(pool* origin) : data(nullptr), origin(origin) {}

            item(pool* origin, T* data) : data(data), origin(origin) {}
        public:
            item() : data(nullptr), origin(nullptr) {}

            item(const item&) = delete;

            item(item&& other) :
                data(std::exchange(other.data, nullptr)),
                origin(std::exchange(other.origin, nullptr))
            {}

            ~item() {
                if (origin) origin->put_back(data);
            }

            auto operator=(const item&) -> item& = delete;

            auto operator=(item&& other) noexcept -> item& {
                data = std::exchange(other.data, nullptr);
                origin = std::exchange(other.origin, nullptr);

                return *this;
            }

            operator T*() const noexcept {
                return data;
            }

            operator T&() const noexcept {
                return *data;
            }

            auto value() const noexcept -> T* {
                return data;
            }
        };

        friend class item;

        struct options final {
            std::size_t initial_size = 1;
            int scale_factor = 1;
            initializer create = []() -> std::unique_ptr<T> {
                static_assert(
                    std::is_default_constructible_v<T>,
                    "Default 'new_item' function requires "
                    "a default constructible type"
                );

                return std::make_unique<T>();
            };
        };

        const options config;

        pool() = default;

        pool(options config) : config(config) {
            expand(config.initial_size);
        }

        pool(const pool&) = delete;

        auto operator=(const pool&) -> pool& = delete;

        auto checkout() -> item {
            if (available.empty()) {
                expand(std::max(
                    storage.size() * config.scale_factor,
                    storage.size() + 1
                ));
            }

            auto* value = available.back();
            available.pop_back();

            return item(this, value);
        }

        auto count_available() const noexcept -> std::size_t {
            return available.size();
        }

        auto count_in_use() const noexcept -> std::size_t {
            return storage.size() - available.size();
        }

        template <typename Callable>
        auto for_each(Callable&& callable) const noexcept -> void {
            for (const auto& item : storage) {
                callable(*item.get());
            }
        }

        auto size() const noexcept -> std::size_t {
            return storage.size();
        }
    };
}
