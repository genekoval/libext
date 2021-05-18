#include <ext/data_size.h>

#include <iomanip>

namespace ext {
    auto operator<<(std::ostream& os, byte_multiple multiple) -> std::ostream& {
        switch (multiple) {
            case byte_multiple::B:   os << "B";   break;
            case byte_multiple::KiB: os << "KiB"; break;
            case byte_multiple::MiB: os << "MiB"; break;
            case byte_multiple::GiB: os << "GiB"; break;
            case byte_multiple::TiB: os << "TiB"; break;
        }

        return os;
    }

    auto data_size::str(uint precision) const -> std::string {
        auto os = std::ostringstream();
        os << std::setprecision(precision) << value << " " << multiple;
        return os.str();
    }
}
