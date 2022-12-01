#pragma once

#include "coroutine/task.hpp"

#include <optional>
#include <vector>

namespace ext {
    namespace detail {
        template <typename T>
        class pool;
    }

    template <typename T>
    class pool_item final {
        friend class detail::pool<T>;

        std::optional<T> item;
        std::optional<std::reference_wrapper<detail::pool<T>>> origin;

        pool_item(T&& item, detail::pool<T>& origin) :
            item(std::forward<T>(item)),
            origin(origin)
        {}
    public:
        pool_item() = default;

        pool_item(const pool_item&) = delete;

        pool_item(pool_item&& other) :
            item(std::exchange(other.item, {})),
            origin(std::exchange(other.origin, {}))
        {}

        ~pool_item() {
            if (!origin) return;

            auto& pool = origin->get();
            pool.checkin(std::move(item).value());
        }

        auto operator=(const pool_item&) -> pool_item& = delete;

        auto operator=(pool_item&& other) noexcept -> pool_item& {
            item = std::exchange(other.item, {});
            origin = std::exchange(other.origin, {});

            return *this;
        }

        auto operator->() const noexcept -> const T* {
            return &*item;
        }

        auto operator->() noexcept -> T* {
            return &*item;
        }

        auto operator*() const& noexcept -> const T& {
            return *item;
        }

        auto operator*() & noexcept -> T& {
            return *item;
        }
    };

    struct pool_options {
        // Use the greatest possible value by default.
        std::size_t max_size = -1;
    };

    namespace detail {
        template <typename T>
        class pool {
            friend class pool_item<T>;

            std::vector<T> storage;
        protected:
            pool_options config;

            auto checkout() -> std::optional<pool_item<T>> {
                if (storage.empty()) return {};

                auto result = make_item(std::move(storage.back()));

                storage.pop_back();
                return result;
            }

            auto make_item(T&& t) -> pool_item<T> {
                return pool_item(std::forward<T>(t), *this);
            }
        public:
            pool() = default;

            pool(pool_options&& config) :
                config(std::forward<pool_options>(config))
            {}

            pool(const pool&) = delete;

            pool(pool&&) = delete;

            auto operator=(const pool&) -> pool& = delete;

            auto operator=(pool&&) -> pool& = delete;

            auto checkin(T&& t) noexcept -> void {
                if (storage.size() < config.max_size) {
                    storage.emplace_back(std::forward<T>(t));
                }
            }

            auto empty() const noexcept -> bool {
                return storage.empty();
            }

            auto max_size() const noexcept -> std::size_t {
                return config.max_size;
            }

            auto size() const noexcept -> std::size_t {
                return storage.size();
            }
        };
    }

    template <typename T, typename Provider>
    concept pool_provider =
        requires(Provider provider) {
            { provider.provide() } -> std::same_as<T>;
        } &&
        requires { Provider{}; };

    template <typename T, typename Provider>
    requires pool_provider<T, Provider>
    class pool final : public detail::pool<T> {
        Provider provider;
    public:
        pool() = default;

        pool(Provider&& provider, pool_options&& config = {}) :
            detail::pool<T>(std::forward<pool_options>(config)),
            provider(std::forward<Provider>(provider))
        {}

        auto checkout() -> pool_item<T> {
            auto cached = detail::pool<T>::checkout();
            if (cached) return std::move(cached).value();

            return detail::pool<T>::make_item(provider.provide());
        }
    };

    template <typename T, typename Provider>
    concept async_pool_provider =
        requires(Provider provider) {
            { provider.provide() } -> std::same_as<task<T>>;
        } &&
        requires { Provider{}; };

    template <typename T, typename Provider>
    requires async_pool_provider<T, Provider>
    class async_pool final : public detail::pool<T> {
        Provider provider;
    public:
        async_pool() = default;

        async_pool(Provider&& provider, pool_options&& config = {}) :
            detail::pool<T>(std::forward<pool_options>(config)),
            provider(std::forward<Provider>(provider))
        {}

        auto checkout() -> task<pool_item<T>> {
            auto cached = detail::pool<T>::checkout();
            if (cached) co_return std::move(cached).value();

            auto t = co_await provider.provide();
            co_return detail::pool<T>::make_item(std::move(t));
        }
    };
}
