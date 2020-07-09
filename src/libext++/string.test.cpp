#include <gtest/gtest.h>

#include <ext/string.h>

using namespace std::literals;

TEST(StringSplit, EmptyString) {
    constexpr auto seq = "";
    auto range = ext::string_range(seq, ":");
    auto it = range.begin();

    ASSERT_EQ("", *it++);
    ASSERT_EQ(range.end(), it);
}

TEST(StringSplit, OneCharDelimiter) {
    constexpr auto seq = "foo*bar";
    auto range = ext::string_range(seq, "*");
    auto it = range.begin();

    ASSERT_EQ("foo", *it++);
    ASSERT_EQ("bar", *it++);
    ASSERT_EQ(range.end(), it);
}

TEST(StringSplit, MultiCharDelimiter) {
    constexpr auto seq = "foo <h> bar <h> baz";
    auto range = ext::string_range(seq, " <h> ");
    auto it = range.begin();

    ASSERT_EQ("foo", *it++);
    ASSERT_EQ("bar", *it++);
    ASSERT_EQ("baz", *it++);
    ASSERT_EQ(range.end(), it);
}

TEST(StringSplit, LeadingTrailingDelimiter) {
    constexpr auto seq = "|hello|world|";
    auto range = ext::string_range(seq, "|");
    auto it = range.begin();

    ASSERT_EQ("", *it++);
    ASSERT_EQ("hello", *it++);
    ASSERT_EQ("world", *it++);
    ASSERT_EQ("", *it++);
    ASSERT_EQ(range.end(), it);
}

TEST(StringSplit, NoDelimiter) {
    constexpr auto seq = "one two three";
    auto range = ext::string_range(seq, "&");
    auto it = range.begin();

    ASSERT_EQ(seq, *it++);
    ASSERT_EQ(range.end(), it);
}

TEST(StringSplit, PartialMatch) {
    constexpr auto seq = "one two three";
    auto range = ext::string_range(seq, "& ");
    auto it = range.begin();

    ASSERT_EQ(seq, *it++);
    ASSERT_EQ(range.end(), it);
}

TEST(StringSplit, Vector) {
    constexpr auto seq = "0 1 2";
    auto vector = ext::split(seq, " ");

    ASSERT_EQ(3, vector.size());
    ASSERT_EQ("0", vector[0]);
    ASSERT_EQ("1", vector[1]);
    ASSERT_EQ("2", vector[2]);
}

TEST(StringSplit, Loop) {
    constexpr auto seq = "one two three";
    constexpr auto array = std::array<decltype(seq), 3> {
        "one",
        "two",
        "three"
    };

    auto index = 0;
    for (const auto token : ext::string_range(seq, " ")) {
        ASSERT_EQ(array[index++], token);
    }
}
