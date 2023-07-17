#pragma once

#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static auto from_json(const json& j, std::optional<T>& opt) -> void {
            if (j.is_null()) opt.reset();
            else opt = j.get<T>();
        }

        static auto to_json(json& j, const std::optional<T>& opt) -> void {
            if (opt) j = *opt;
            else j = nullptr;
        }
    };

    template <typename Duration>
    struct adl_serializer<
        std::chrono::time_point<std::chrono::system_clock, Duration>
    > {
        using clock = std::chrono::system_clock;
        using time_point = std::chrono::time_point<clock, Duration>;

        static auto from_json(const json& j, time_point& t) -> void {
            using std::chrono::milliseconds;

            const auto string = j.get<std::string>();
            const auto input = string.substr(0, string.size() - 5);

            auto stream = std::istringstream(input);
            auto tm = std::tm();

            stream.imbue(std::locale::classic());
            stream >> std::get_time(&tm, "%Y-%m-%dT%T");

            if (stream.fail()) {
                throw std::runtime_error("Failed to parse JSON timestamp");
            }

            const auto ms_str = string.substr(string.size() - 4, 3);
            const auto ms = milliseconds(std::stoi(std::string(ms_str)));

            auto time = clock::from_time_t(timegm(&tm));
            time += ms;

            t = std::chrono::time_point_cast<Duration>(time);
        }

        static auto to_json(json& j, time_point t) -> void {
            using namespace std::chrono;

            const auto time = clock::to_time_t(t);

            auto tm = std::tm();
            gmtime_r(&time, &tm);

            const auto epoch = t.time_since_epoch();
            const auto millis =
                duration_cast<milliseconds>(epoch).count() % 1000;

            auto stream = std::ostringstream();
            stream.imbue(std::locale::classic());

            stream
                << std::put_time(&tm, "%FT%T.")
                << std::setw(3) << std::setfill('0') << millis << "Z";

            j = stream.str();
        }
    };
}
