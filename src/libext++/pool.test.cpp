#include <ext/detail/pool.hpp>

#include <gtest/gtest.h>

using ext::pool_options;

namespace {
    class provider final {
        int counter = 0;
    public:
        auto provide() -> int {
            return counter++;
        }
    };

    using int_pool = ext::pool<provider>;

    static_assert(ext::pool_provider_sync<provider>);
    static_assert(std::is_same_v<int_pool::value_type, int>);
}

class PoolTest : public testing::Test {
protected:
    int_pool pool;

    PoolTest() : pool(pool_options { .max_size = 2 }) {}
};

TEST_F(PoolTest, Checkout) {
    EXPECT_TRUE(pool.empty());

    {
        const auto item = pool.checkout();
        EXPECT_EQ(0, *item);
    }

    EXPECT_EQ(1, pool.size());
}

TEST_F(PoolTest, MaxSize) {
    {
        const auto zero = pool.checkout();
        EXPECT_EQ(0, *zero);

        const auto one = pool.checkout();
        EXPECT_EQ(1, *one);

        const auto two = pool.checkout();
        EXPECT_EQ(2, *two);
    }

    EXPECT_EQ(2, pool.size());
}

TEST_F(PoolTest, Reuse) {
    {
        const auto zero = pool.checkout();
        EXPECT_EQ(0, *zero);
    }

    const auto zero = pool.checkout();
    EXPECT_EQ(0, *zero);
}
