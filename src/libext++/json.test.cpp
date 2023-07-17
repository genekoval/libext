#include <ext/json.hpp>

#include <gtest/gtest.h>

using namespace std::literals;

using json = nlohmann::json;

TEST(Json, TimePoint) {
    using namespace std::chrono;

    using time_point_millis = time_point<system_clock, milliseconds>;

    constexpr auto time = time_point_millis(1689466168033ms);
    constexpr auto string = "2023-07-16T00:09:28.033Z"sv;

    const auto json_string = json(string);

    EXPECT_EQ(json_string, json(time));

    const auto from_json = json_string.get<time_point_millis>();

    EXPECT_EQ(time, from_json);
}
