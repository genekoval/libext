#include <nova/ext/chrono.h>

#include <ctime>
#include <iomanip>
#include <stdexcept>

using nova::ext::chrono::time_point;
using nova::ext::chrono::time_type;
using nova::ext::chrono::timestamp;
using std::runtime_error;
using std::string;
using std::string_view;
using std::ostream;
using std::tm;

static tm* to_tm(const timestamp& ts) {
    using namespace nova::ext::chrono;

    auto time = clock::to_time_t(ts.time());

    switch (ts.type()) {
        case gmt:
            return std::gmtime(&time);
        case local:
            return std::localtime(&time);
        default:
            throw runtime_error("Unhandled 'time_type' enumerator");
    }
}

timestamp::timestamp(string_view format, time_type type) :
    m_format(format),
    m_time(clock::now()),
    m_type(type)
{}

string timestamp::format() const { return m_format; }

std::string timestamp::strftime(int buffer_size) const {
    char buffer[buffer_size];
    auto bytes_written =
        std::strftime(buffer, buffer_size, m_format.c_str(), to_tm(*this));

    if (bytes_written == 0) return "";
    return buffer;
}

time_point timestamp::time() const { return m_time; }

time_type timestamp::type() const { return m_type; }

ostream& nova::ext::chrono::operator<<(ostream& os, const timestamp& ts) {
    os << std::put_time(to_tm(ts), ts.format().c_str());
    return os;
}
