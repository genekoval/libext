#include <ext/detail/async_pool.hpp>

#include <gtest/gtest.h>

namespace {
    class provider {
        int counter = 0;
    public:
        auto provide() -> ext::task<int> {
            co_return counter++;
        }
    };

    using int_pool = ext::async_pool<int, provider>;

    auto make_pool() -> int_pool {
        return int_pool(int_pool::options {
            .provider = provider()
        });
    }
}

TEST(AsyncPool, Checkout) {
    []() -> ext::detached_task {
        auto pool = make_pool();

        EXPECT_TRUE(pool.empty());

        {
            const auto item = co_await pool.checkout();
            EXPECT_EQ(0, item.value());
        }

        EXPECT_EQ(1, pool.size());
    }();
}

TEST(AsyncPool, MaxSize) {
    []() -> ext::detached_task {
        auto pool = int_pool(int_pool::options {
            .max_size = 2,
            .provider = provider()
        });

        {
            const auto zero = co_await pool.checkout();
            EXPECT_EQ(0, zero.value());

            const auto one = co_await pool.checkout();
            EXPECT_EQ(1, one.value());

            const auto two = co_await pool.checkout();
            EXPECT_EQ(2, two.value());
        }

        EXPECT_EQ(2, pool.size());
    }();
}

TEST(AsyncPool, Reuse) {
    []() -> ext::detached_task {
        auto pool = make_pool();

        {
            const auto zero = co_await pool.checkout();
            EXPECT_EQ(0, zero.value());
        }

        const auto zero = co_await pool.checkout();
        EXPECT_EQ(0, zero.value());
    }();
}
