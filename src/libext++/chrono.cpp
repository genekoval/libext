#include <ext/chrono.h>

#include <ctime>
#include <iomanip>
#include <stdexcept>

namespace ext::chrono {
    static auto to_tm(const timestamp& ts) -> std::tm* {
        auto time = clock::to_time_t(ts.time());

        switch (ts.type()) {
            case gmt:
                return std::gmtime(&time);
            case local:
                return std::localtime(&time);
            default:
                throw std::runtime_error("Unhandled 'time_type' enumerator");
        }
    }

    timestamp::timestamp(std::string_view format, time_type type) :
        m_format(format),
        m_time(clock::now()),
        m_type(type)
    {}

    auto timestamp::format() const -> std::string { return m_format; }

    auto timestamp::strftime(int buffer_size) const -> std::string {
        char buffer[buffer_size];
        auto bytes_written =
            std::strftime(buffer, buffer_size, m_format.c_str(), to_tm(*this));

        if (bytes_written == 0) return "";
        return buffer;
    }

    auto timestamp::time() const -> time_point { return m_time; }

    auto timestamp::type() const -> time_type { return m_type; }

    auto operator<<(
        std::ostream& os,
        const timestamp& ts
    ) -> std::ostream& {
        os << std::put_time(to_tm(ts), ts.format().c_str());
        return os;
    }
}