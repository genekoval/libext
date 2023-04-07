#include <ext/detail/coroutine/detached_task.hpp>
#include <ext/detail/coroutine/jtask.hpp>

#include <gtest/gtest.h>

using namespace std::literals;

using ext::detached_task;
using ext::jtask;

namespace {
    constexpr auto fstr = "f()"sv;

    struct test_error : std::exception {
        auto what() const noexcept -> const char* override {
            return "test error";
        }
    };

    auto e() -> jtask<> {
        throw test_error();
        co_return;
    }

    template <typename Container>
    requires std::convertible_to<
        std::string_view,
        typename Container::value_type
    >
    auto f(std::back_insert_iterator<Container> it) -> jtask<> {
        it = fstr;
        co_return;
    }

    auto g(int x) -> jtask<int> {
        co_return x + 1;
    }
}

TEST(JTask, JoinVoid) {
    constexpr auto before = "before join"sv;
    constexpr auto after = "after join"sv;

    auto results = std::vector<std::string_view>();
    auto it = std::back_inserter(results);

    [&]() -> detached_task {
        auto handle = f(it);
        it = "before join"sv;
        co_await handle;
        it = "after join"sv;
    }();

    EXPECT_EQ(fstr, results.at(0));
    EXPECT_EQ(before, results.at(1));
    EXPECT_EQ(after, results.at(2));
}

TEST(JTask, JoinValue) {
    []() -> detached_task {
        auto handle = g(42);
        EXPECT_EQ(43, co_await handle);
    }();
}

TEST(JTask, JoinException) {
    []() -> detached_task {
        auto handle = e();
        EXPECT_THROW(co_await handle, test_error);
    }();
}

TEST(JTask, Joinable) {
    const auto t1 = jtask();
    EXPECT_FALSE(t1.joinable());

    const auto t2 = g(0);
    EXPECT_TRUE(t2.joinable());
}
