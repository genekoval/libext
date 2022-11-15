#include <ext/detail/coroutine/generator.hpp>

#include <gtest/gtest.h>

namespace {
    auto counter(unsigned int max = -1) -> ext::generator<unsigned int> {
        auto n = 0u;

        while (n < max) {
            const auto result = n++;
            co_yield result;
        }
    }
}

TEST(Generator, Counter) {
    auto gen = counter(3);

    EXPECT_EQ(0, gen());
    EXPECT_EQ(1, gen());
    EXPECT_EQ(2, gen());

    EXPECT_FALSE(gen);
}

TEST(Generator, Bool) {
    {
        auto gen = counter();

        EXPECT_TRUE(static_cast<bool>(gen));
        EXPECT_EQ(0, gen());
    }

    {
        auto gen = counter();

        EXPECT_EQ(0, gen());
        EXPECT_TRUE(static_cast<bool>(gen));
    }
}

TEST(Generator, Loop) {
    auto gen = counter(3);
    auto values = std::vector<unsigned int>();

    while (gen) values.push_back(gen());

    EXPECT_EQ(3, values.size());

    EXPECT_EQ(0, values.at(0));
    EXPECT_EQ(1, values.at(1));
    EXPECT_EQ(2, values.at(2));
}
