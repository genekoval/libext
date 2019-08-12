#pragma once

#include <chrono>
#include <sstream>
#include <string>
#include <string_view>

namespace nova::ext::chrono {
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

        std::string format() const;
        std::string strftime(int buffer_size) const;
        time_point time() const;
        time_type type() const;
    };

    std::ostream& operator<<(std::ostream& os, const timestamp& ts);
}
