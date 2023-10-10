#include <ext/dynarray>

#include <cstring>
#include <gtest/gtest.h>
#include <span>

using namespace std::literals;

using ext::dynarray;

namespace {
    class foo {
        bool* alive;
    public:
        foo(bool& alive) : alive(&alive) { *this->alive = true; }

        foo(const foo&) = delete;

        foo(foo&&) = delete;

        ~foo() { *this->alive = false; }

        auto operator=(const foo&) -> foo& = delete;

        auto operator=(foo&&) -> foo& = delete;
    };
}

TEST(Dynarray, DefaultConstruction) {
    const auto array = dynarray<int>();

    EXPECT_TRUE(array.empty());
    EXPECT_EQ(0, array.capacity());
    EXPECT_EQ(0, array.size());
    EXPECT_EQ(nullptr, array.data());
}

TEST(Dynarray, CapacityConstruction) {
    constexpr dynarray<int>::size_type capacity = 3;

    const auto array = dynarray<int>(capacity);

    EXPECT_TRUE(array.empty());
    EXPECT_EQ(capacity, array.capacity());
    EXPECT_EQ(0, array.size());
    EXPECT_NE(nullptr, array.data());
}

TEST(Dynarray, InitializerListConstruction) {
    const dynarray<int> array = {5, 10, 15, 20};

    EXPECT_FALSE(array.empty());
    EXPECT_EQ(4, array.capacity());
    EXPECT_EQ(array.capacity(), array.size());
    EXPECT_NE(nullptr, array.data());
}

TEST(Dynarray, EmplaceBack) {
    auto array = dynarray<foo>(1);
    auto alive = false;

    array.emplace_back(alive);
    EXPECT_TRUE(alive);

    EXPECT_FALSE(array.empty());
    EXPECT_EQ(1, array.capacity());
    EXPECT_EQ(1, array.size());

    array.pop_back();
    EXPECT_FALSE(alive);

    EXPECT_TRUE(array.empty());
    EXPECT_EQ(1, array.capacity());
    EXPECT_EQ(0, array.size());
}

TEST(Dynarray, ElementAccess) {
    auto array = dynarray<int>(3);

    array.emplace_back(10);
    array.emplace_back(20);
    array.emplace_back(30);

    EXPECT_EQ(10, array[0]);
    EXPECT_EQ(20, array[1]);
    EXPECT_EQ(30, array[2]);
}

TEST(Dynarray, Clear) {
    auto array = dynarray<int>(3);

    array.emplace_back(10);
    array.emplace_back(20);
    array.emplace_back(30);

    array.clear();

    EXPECT_TRUE(array.empty());
    EXPECT_EQ(3, array.capacity());
    EXPECT_EQ(0, array.size());
    EXPECT_NE(nullptr, array.data());
}

TEST(Dynarray, Data) {
    const auto array = dynarray<int> {2, 4, 8};

    const auto* const data = array.data();

    EXPECT_EQ(2, *data);
    EXPECT_EQ(4, *(data + 1));
    EXPECT_EQ(8, *(data + 2));
}

TEST(Dynarray, ConstIterator) {
    const auto array = dynarray<int> {1, 2, 3};

    auto it = array.begin();
    const auto end = array.end();

    EXPECT_EQ(1, *it++);
    EXPECT_EQ(2, *it++);
    EXPECT_EQ(3, *it++);
    EXPECT_EQ(end, it);

    auto i = 1;
    for (const auto& item : array) EXPECT_EQ(i++, item);
}

TEST(Dynarray, FrontBack) {
    const auto array = dynarray<int> {1, 2, 3};

    const auto& front = array.front();
    const auto& back = array.back();

    EXPECT_EQ(1, front);
    EXPECT_EQ(3, back);
}

TEST(Dynarray, Span) {
    const auto array = dynarray<int> {100, 200, 300};
    const std::span<const int> span = array;

    ASSERT_EQ(3, span.size());
    EXPECT_EQ(100, span[0]);
    EXPECT_EQ(200, span[1]);
    EXPECT_EQ(300, span[2]);
}
