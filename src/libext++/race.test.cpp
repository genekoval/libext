#include <ext/coroutine>

#include <gtest/gtest.h>

using ext::jtask;
using ext::make_task;
using ext::race;

TEST(Race, ReturnValue) {
    []() -> jtask<> {
        auto result =
            co_await race(make_task(42), make_task<std::string>("Hello"));

        EXPECT_EQ(0, result.index());
        EXPECT_EQ(42, co_await std::get<0>(std::move(result)));
    }()
                .result();
}

TEST(Race, ThrowException) {
    []() -> jtask<> {
        auto result = co_await race([]() -> ext::task<> {
            throw std::runtime_error("Test error");
            co_return;
        }());

        EXPECT_THROW(
            co_await std::get<0>(std::move(result)),
            std::runtime_error
        );
    }()
                .result();
}

TEST(Race, EarlyExit) {
    []() -> jtask<> {
        auto first_started = false;
        auto second_started = false;

        auto task = [](bool& completed) -> ext::task<> {
            completed = true;
            co_return;
        };

        co_await race(task(first_started), task(second_started));

        EXPECT_TRUE(first_started);
        EXPECT_FALSE(second_started);
    }()
                .result();
}
