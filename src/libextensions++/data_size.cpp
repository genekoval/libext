#include <ext/data_size.h>

#include <array> // std::array
#include <iomanip> // std::setprecision

namespace ext {
    static constexpr auto unit_max = 1024u;
    static constexpr auto precision = 3;
    static const auto multiples = std::array<byte_multiple, 5>{
        B, KiB, MiB, GiB, TiB
    };

    data_size::data_size(long double size, byte_multiple multiple) :
        size(size),
        multiple(multiple)
    {}

    auto data_size::convert_to(
        long double size,
        byte_multiple multiple
    ) -> data_size {
        auto i = 0u;

        while (size >= unit_max && multiples[i] != multiple) {
            size /= unit_max;
            i++;
        }

        return data_size(size, multiples[i]);
    }

    auto data_size::format(uintmax_t bytes) -> data_size {
        return convert_to(bytes, multiples[multiples.size() - 1]);
    }

    auto operator<<(std::ostream& os, byte_multiple multiple) -> std::ostream& {
        switch (multiple) {
            case   B: os <<   "B"; break;
            case KiB: os << "KiB"; break;
            case MiB: os << "MiB"; break;
            case GiB: os << "GiB"; break;
            case TiB: os << "TiB"; break;
        }

        return os;
    }

    auto operator<<(std::ostream& os, const data_size& ds) -> std::ostream& {
        os
            << std::setprecision(precision) << ds.size
            << " "
            << ds.multiple;

        return os;
    }

    auto to_string(const data_size& ds) -> std::string {
        std::ostringstream os;
        os << ds;
        return os.str();
    }
}
