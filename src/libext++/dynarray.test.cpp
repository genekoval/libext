#include <ext/dynarray.h>

#include <cstring>
#include <gtest/gtest.h>

using namespace std::literals;

TEST(Dynarray, CopyData) {
    constexpr auto text = "Test data."sv;

    const auto array = ext::dynarray<char>(text.size());
    array.copy(text.data(), text.size());

    const auto span = array.span();

    for (auto i = 0ul; i < text.size(); ++i) {
        ASSERT_EQ(text[i], span[i]);
    }
}

TEST(Dynarray, CopyWithOffset) {
    constexpr auto text = "Test data."sv;
    constexpr auto offset = text.size() / 2;
    constexpr auto remainder = text.size() - offset;

    const auto array = ext::dynarray<char>(text.size());
    array.copy(text.data(), offset);
    array.copy(&text[offset], remainder, offset);

    const auto span = array.span();

    for (auto i = 0ul; i < text.size(); ++i) {
        ASSERT_EQ(text[i], span[i]);
    }
}

TEST(Dynarray, Create) {
    constexpr auto size = 10;

    const auto array = ext::dynarray<int>(size);

    ASSERT_NE(nullptr, array.data());
    ASSERT_EQ(size, array.size());
}

TEST(Dynarray, CreateEmpty) {
    const auto array = ext::dynarray<int>();

    ASSERT_EQ(nullptr, array.data());
    ASSERT_EQ(0ul, array.size());
}

TEST(Dynarray, Release) {
    constexpr auto text = "Hello, world!"sv;

    auto array = ext::dynarray<char>(text.size());
    array.copy(text.data(), text.size());

    auto* ptr = array.release();

    ASSERT_EQ(nullptr, array.data());
    ASSERT_EQ(0ul, array.size());

    const auto span = array.span();

    ASSERT_EQ(nullptr, span.data());
    ASSERT_EQ(0ul, span.size());

    const auto data = std::span(ptr, text.size());

    for (auto i = 0ul; i < text.size(); ++i) {
        ASSERT_EQ(text[i], data[i]);
    }

    delete[] ptr;
}
