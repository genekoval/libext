#pragma once

#include <chrono>
#include <sstream>
#include <string>
#include <string_view>

namespace ext::chrono {
    using clock = std::chrono::system_clock;
    using time_point = clock::time_point;

    enum time_type {
        gmt,
        local
    };

    class timestamp {
        std::string m_format;
        time_point m_time;
        time_type m_type;
    public:
        timestamp(std::string_view format, time_type type);

        auto format() const -> std::string;
        auto strftime(int buffer_size) const -> std::string;
        auto time() const -> time_point;
        auto type() const -> time_type;
    };

    auto operator<<(std::ostream& os, const timestamp& ts) -> std::ostream&;
}
