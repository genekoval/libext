#pragma once

#include "coroutine/task.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace ext {
    struct pool_options {
        // Use the greatest possible value by default.
        std::size_t max_size = -1;
    };

    namespace detail {
        template <typename T>
        struct pool_value {
            using type = T;
        };

        template <typename T>
        struct pool_value<ext::task<T>> {
            using type = T;
        };
    };

    template <typename Provider>
    struct pool_value {
        using type = typename detail::pool_value<
            decltype(std::declval<Provider>().provide())
        >::type;
    };

    template <typename Provider>
    concept pool_provider = requires(Provider&& provider) {
        { provider.provide() };
    };

    template <typename Provider>
    concept pool_provider_checkin = requires(
        Provider provider,
        typename pool_value<Provider>::type& t
    ) {
        { provider.checkin(t) } -> std::same_as<bool>;
    };

    template <typename Provider>
    concept pool_provider_checkout = requires(
        Provider provider,
        typename pool_value<Provider>::type& t
    ) {
        { provider.checkout(t) } -> std::same_as<bool>;
    };

    template <typename Provider>
    concept pool_provider_async = requires(Provider provider) {
        { provider.provide() } ->
            std::same_as<ext::task<typename pool_value<Provider>::type>>;
    };

    template <typename Provider>
    concept pool_provider_sync = requires(Provider provider) {
        { provider.provide() } ->
            std::same_as<typename pool_value<Provider>::type>;
    };

    template <pool_provider Provider>
    struct pool;

    namespace detail {
        template <typename T>
        struct pool;

        template <typename T, typename Provider>
        struct pool_type {
            using type = ext::pool<Provider>;
        };

        template <typename T>
        struct pool_type<T, void> {
            using type = pool<T>;
        };
    }

    template <typename T, typename Provider = void>
    class pool_item final {
        using pool = typename detail::pool_type<T, Provider>::type;

        std::optional<T> storage;
        pool* origin = nullptr;
    public:
        pool_item() = default;

        pool_item(T&& t, pool& origin) :
            storage(std::forward<T>(t)),
            origin(&origin)
        {}

        pool_item(const pool_item&) = delete;

        pool_item(pool_item&& other) :
            storage(std::exchange(other.storage, std::nullopt)),
            origin(std::exchange(other.origin, nullptr))
        {}

        ~pool_item() {
            checkin();
        }

        auto operator=(const pool_item&) -> pool_item& = delete;

        auto operator=(pool_item&& other) -> pool_item& {
            if (std::addressof(other) != this) {
                checkin();

                storage = std::exchange(other.storage, std::nullopt);
                origin = std::exchange(other.origin, nullptr);
            }

            return *this;
        }

        auto operator->() const noexcept -> const T* {
            return &*storage;
        }

        auto operator->() noexcept -> T* {
            return &*storage;
        }

        auto operator*() const& noexcept -> const T& {
            return *storage;
        }

        auto operator*() & noexcept -> T& {
            return *storage;
        }

        explicit operator bool() const noexcept {
            return has_value();
        }

        auto checkin() noexcept -> void {
            if (!origin) return;

            origin->checkin(*std::exchange(storage, std::nullopt));
            origin = nullptr;
        }

        auto has_value() const noexcept -> bool {
            return storage.has_value();
        }

        auto release() noexcept -> std::optional<T> {
            origin = nullptr;
            return std::exchange(storage, std::nullopt);
        }

        auto reset() noexcept -> void {
            storage.reset();
            origin = nullptr;
        }
    };

    namespace detail {
        template <typename T, typename Provider>
        struct item_type {
            using type = pool_item<T, void>;
        };

        template <typename T, pool_provider_checkin Provider>
        struct item_type<T, Provider> {
            using type = pool_item<T, Provider>;
        };

        template <typename T>
        struct pool {
            const pool_options config;

            friend pool_item<T>;
        protected:
            std::vector<T> storage;

            pool() = default;

            pool(const pool_options& config) : config(config) {}
        private:
            auto checkin(T&& t) noexcept -> void {
                if (storage.size() < config.max_size) {
                    storage.emplace_back(std::forward<T>(t));
                }
            }
        };
    }

    template <pool_provider Provider>
    struct pool final :
        detail::pool<typename pool_value<Provider>::type>
    {
        using value_type = typename pool_value<Provider>::type;
        using item = typename detail::item_type<value_type, Provider>::type;

        friend item;

        Provider provider;
    private:
        auto checkin(value_type&& t) noexcept -> void {
            if (
                this->storage.size() < this->config.max_size &&
                provider.checkin(t)
            ) this->storage.emplace_back(std::forward<value_type>(t));
        }

        auto try_checkout() -> std::optional<item> {
            while (!this->storage.empty()) {
                if constexpr (pool_provider_checkout<Provider>) {
                    if (!provider.checkout(this->storage.back())) {
                        this->storage.pop_back();
                        continue;
                    }
                }

                auto result = item(std::move(this->storage.back()), *this);
                this->storage.pop_back();
                return result;
            }

            return std::nullopt;
        }
    public:
        pool() = default;

        template <typename... Args>
        pool(const pool_options& config, Args&&... args) :
            detail::pool<value_type>(config),
            provider(std::forward<Args>(args)...)
        {}

        pool(const pool&) = delete;

        pool(pool&&) = delete;

        auto operator=(const pool&) -> pool& = delete;

        auto operator=(pool&&) -> pool& = delete;

        auto checkout() -> item requires pool_provider_sync<Provider> {
            if (auto item = try_checkout()) return std::move(*item);
            return item(provider.provide(), *this);
        }

        auto checkout() -> ext::task<item>
        requires pool_provider_async<Provider> {
            if (auto item = try_checkout()) co_return std::move(*item);
            co_return item(co_await provider.provide(), *this);
        }

        auto empty() const noexcept -> bool {
            return this->storage.empty();
        }

        auto size() const noexcept -> std::size_t {
            return this->storage.size();
        }
    };
}
