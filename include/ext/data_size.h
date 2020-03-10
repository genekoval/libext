#pragma once

#include <ostream> // std::ostream
#include <string> // std::string

namespace ext {
    enum byte_multiple {
        B, KiB, MiB, GiB, TiB
    };

    auto operator<<(std::ostream& os, byte_multiple multiple) -> std::ostream&;

    class data_size {
        static auto convert_to(
            long double size,
            byte_multiple multiple
        ) -> data_size;

        const long double size;
        const byte_multiple multiple;

        data_size(long double size, byte_multiple multiple);
    public:
        friend
            auto operator<<(
                std::ostream& os,
                const data_size& size
            ) -> std::ostream&;

        static auto format(uintmax_t bytes) -> data_size;
    };

    auto to_string(const data_size& ds) -> std::string;
}
