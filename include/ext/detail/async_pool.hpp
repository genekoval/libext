#pragma once

#include "coroutine/task.hpp"

#include <vector>

namespace ext {
    template <typename T, typename Provider>
    concept async_pool_provider =
        requires(Provider provider) {
            { provider.provide() } -> std::same_as<task<T>>;
        } &&
        requires { Provider{}; };

    template <typename T, typename Provider>
    requires async_pool_provider<T, Provider>
    class async_pool final {
    public:
        struct options final {
            // Use the greatest possible value by default.
            std::size_t max_size = -1;
            Provider provider;
        };
    private:
        options config;
        std::vector<T> storage;

        auto checkin(T&& t) -> void {
            if (storage.size() < config.max_size) {
                storage.emplace_back(std::forward<T>(t));
            }
        }
    public:
        class item final {
            friend class async_pool;

            T t;
            async_pool* origin;

            item(T&& t, async_pool* origin) :
                t(std::forward<T>(t)),
                origin(origin)
            {}
        public:
            item() : origin(nullptr) {}

            item(const item&) = delete;

            item(item&& other) :
                t(std::move(other.t)),
                origin(std::exchange(other.origin, nullptr))
            {}

            ~item() {
                if (origin) origin->checkin(std::move(t));
            }

            auto operator=(const item&) -> item& = delete;

            auto operator=(item&& other) noexcept -> item& {
                t = std::move(other.t);
                origin = std::exchange(other.origin, nullptr);

                return *this;
            }

            operator T*() const noexcept {
                return &t;
            }

            operator T&() const noexcept {
                return t;
            }

            auto value() noexcept -> T& {
                return t;
            }

            auto value() const noexcept -> const T& {
                return t;
            }
        };

        friend class item;

        async_pool() = default;

        async_pool(options&& config) : config(std::move(config)) {}

        async_pool(const async_pool&) = delete;

        auto operator=(const async_pool&) -> async_pool& = delete;

        auto checkout() -> task<item> {
            if (!storage.empty()) {
                auto result = item(std::move(storage.back()), this);
                storage.pop_back();
                co_return result;
            }

            auto t = co_await config.provider.provide();
            auto result = item(std::move(t), this);

            co_return result;
        }

        auto empty() const noexcept -> bool {
            return storage.empty();
        }

        auto max_size() const noexcept -> std::size_t {
            return config.max_size;
        }

        auto provider() const noexcept -> Provider& {
            return config.provider;
        }

        auto size() const noexcept -> std::size_t {
            return storage.size();
        }
    };
}
