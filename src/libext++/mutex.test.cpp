#include <ext/coroutine>

#include <gtest/gtest.h>

using task = ext::jtask<>;

TEST(Mutex, SingleLock) {
    []() -> task {
        auto mutex = ext::mutex();

        EXPECT_FALSE(mutex.locked());
        EXPECT_EQ(0, mutex.queue_size());

        {
            const auto lock = co_await mutex.lock();

            EXPECT_TRUE(mutex.locked());
            EXPECT_EQ(0, mutex.queue_size());
        }

        EXPECT_FALSE(mutex.locked());
        EXPECT_EQ(0, mutex.queue_size());
    }().result();
}

TEST(Mutex, MultiLock) {
    []() -> task {
        auto mutex = ext::mutex();
        auto counter = ext::counter();

        auto ids = std::vector<int>();

        const auto start = [&](int id) -> ext::detached_task {
            counter.increment();
            const auto lock = co_await mutex.lock();

            ids.emplace_back(id);
        };

        {
            const auto lock = co_await mutex.lock();
            for (auto i = 0; i < 5; ++i) start(i);
        }

        co_await counter.await();

        EXPECT_EQ(5, ids.size());

        for (auto i = 0; i < ids.size(); ++i)  {
            EXPECT_EQ(i, ids.at(i));
        }
    }().result();
}
