#include <ext/coroutine>

#include <gtest/gtest.h>

using task = ext::jtask<>;

TEST(Mutex, SingleLock) {
    []() -> task {
        auto mutex = ext::mutex();

        EXPECT_TRUE(mutex.unlocked());
        EXPECT_EQ(0, mutex.queue_size());

        {
            const auto lock = co_await mutex.lock();

            EXPECT_FALSE(mutex.unlocked());
            EXPECT_EQ(0, mutex.queue_size());
        }

        EXPECT_TRUE(mutex.unlocked());
        EXPECT_EQ(0, mutex.queue_size());
    }()
                .result();
}

TEST(Mutex, MultiLock) {
    []() -> task {
        auto mutex = ext::mutex();
        auto ids = std::vector<int>();

        const auto start = [&](int id) -> ext::detached_task {
            const auto lock = co_await mutex.lock();
            ids.emplace_back(id);
        };

        {
            const auto lock = co_await mutex.lock();
            for (auto i = 0; i < 5; ++i) start(i);

            EXPECT_EQ(5, mutex.queue_size());
        }

        EXPECT_TRUE(mutex.unlocked());
        EXPECT_EQ(5, ids.size());

        for (auto i = 0; i < ids.size(); ++i) { EXPECT_EQ(i, ids.at(i)); }
    }()
                .result();
}
