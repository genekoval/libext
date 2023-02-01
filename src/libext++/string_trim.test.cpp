#include <ext/string.h>

#include <gtest/gtest.h>

using namespace std::literals;

TEST(StringTrim, TrimStart) {
    EXPECT_EQ("foo"sv, ext::trim_start("foo"));
    EXPECT_EQ("foo"sv, ext::trim_start(" foo"));
    EXPECT_EQ("foo"sv, ext::trim_start("   \n\t  foo"));
    EXPECT_EQ("foo bar"sv, ext::trim_start("foo bar"));
    EXPECT_EQ("foo bar"sv, ext::trim_start(" foo bar"));
    EXPECT_EQ("foo "sv, ext::trim_start(" foo "));
}

TEST(StringTrim, TrimEnd) {
    EXPECT_EQ("foo"sv, ext::trim_end("foo"));
    EXPECT_EQ("foo"sv, ext::trim_end("foo "));
    EXPECT_EQ("foo"sv, ext::trim_end("foo   \n  \t   "));
    EXPECT_EQ("foo bar"sv, ext::trim_end("foo bar"));
    EXPECT_EQ("foo bar"sv, ext::trim_end("foo bar "));
    EXPECT_EQ(" foo"sv, ext::trim_end(" foo "));
}
