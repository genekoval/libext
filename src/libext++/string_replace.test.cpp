#include <gtest/gtest.h>

#include <ext/string.h>

using namespace std::literals;

TEST(StringReplace, ReplaceWord) {
    constexpr auto sequence = "Quick brown fox.";
    constexpr auto expected = "Quick blue fox.";

    auto result = ext::replace(
        sequence,
        std::regex("brown"),
        [](const std::cmatch&) -> std::string {
            return "blue";
        }
    );

    ASSERT_EQ(expected, result);
}

TEST(StringReplace, EnvironmentVariables) {
    constexpr auto var = "$HOME";
    const auto expected = std::string(std::getenv("HOME"));

    ASSERT_EQ(expected, ext::expand_env(var));
}
