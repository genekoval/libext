#include <ext/detail/pool.hpp>

#include <gtest/gtest.h>

namespace {
    class provider final {
        int counter = 0;
    public:
        auto provide() -> ext::task<int> {
            co_return counter++;
        }
    };

    using int_pool = ext::async_pool<int, provider>;
}

class AsyncPoolTest : public testing::Test {
protected:
    int_pool pool;

    AsyncPoolTest() : pool(provider(), ext::pool_options { .max_size = 2 }) {}
};

TEST_F(AsyncPoolTest, Checkout) {
    [this]() -> ext::detached_task {
        EXPECT_TRUE(pool.empty());

        {
            const auto item = co_await pool.checkout();
            EXPECT_EQ(0, *item);
        }

        EXPECT_EQ(1, pool.size());
    }();
}

TEST_F(AsyncPoolTest, MaxSize) {
    [this]() -> ext::detached_task {
        {
            const auto zero = co_await pool.checkout();
            EXPECT_EQ(0, *zero);

            const auto one = co_await pool.checkout();
            EXPECT_EQ(1, *one);

            const auto two = co_await pool.checkout();
            EXPECT_EQ(2, *two);
        }

        EXPECT_EQ(2, pool.size());
    }();
}

TEST_F(AsyncPoolTest, Reuse) {
    [this]() -> ext::detached_task {
        {
            const auto zero = co_await pool.checkout();
            EXPECT_EQ(0, *zero);
        }

        const auto zero = co_await pool.checkout();
        EXPECT_EQ(0, *zero);
    }();
}
