#pragma once

#include <array>
#include <cmath>
#include <ostream>
#include <string>

namespace ext {
    constexpr auto byte_unit_max = 1'024u;

    enum class byte_multiple {
        B, KiB, MiB, GiB, TiB
    };

    constexpr auto multiples = std::array<byte_multiple, 5> {
        byte_multiple::B,
        byte_multiple::KiB,
        byte_multiple::MiB,
        byte_multiple::GiB,
        byte_multiple::TiB
    };

    auto operator<<(std::ostream& os, byte_multiple multiple) -> std::ostream&;

    class data_size {
        static constexpr auto pow(int base, int exp) -> long {
            return exp ? base * pow(base, exp - 1) : 1;
        }

        constexpr data_size(
            uintmax_t bytes,
            double value,
            byte_multiple multiple
        ) :
            bytes(bytes),
            value(value),
            multiple(multiple)
        {}
    public:
        static constexpr auto format(uintmax_t bytes) -> data_size {
            if (bytes == 0) return data_size();

            const auto magnitude = static_cast<int>(
                std::log(bytes) / std::log(byte_unit_max)
            );
            const auto value = static_cast<double>(
                bytes) / (1L << (magnitude * 10)
            );

            try {
                return data_size(bytes, value, multiples.at(magnitude));
            }
            catch (const std::out_of_range&) {
                throw std::runtime_error(
                    std::string("unknown magnitude: ") +
                    std::to_string(magnitude)
                );
            }
        }

        const uintmax_t bytes = 0;
        const double value = 0.0;
        const byte_multiple multiple = ext::byte_multiple::B;

        constexpr data_size() = default;

        constexpr data_size(double value, byte_multiple multiple) :
            bytes(
                pow(
                    byte_unit_max,
                    static_cast<std::underlying_type_t<byte_multiple>>(multiple)
                ) * value
            ),
            value(value),
            multiple(multiple)
        {}

        auto str(uint decimal_places) const -> std::string;
    };

    namespace literals {
        constexpr auto operator""_KiB(
            unsigned long long int value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::KiB).bytes;
        }

        constexpr auto operator""_MiB(
            unsigned long long int value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::MiB).bytes;
        }

        constexpr auto operator""_GiB(
            unsigned long long int value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::GiB).bytes;
        }

        constexpr auto operator""_TiB(
            unsigned long long int value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::TiB).bytes;
        }

        constexpr auto operator""_KiB(
            long double value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::KiB).bytes;
        }

        constexpr auto operator""_MiB(
            long double value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::MiB).bytes;
        }

        constexpr auto operator""_GiB(
            long double value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::GiB).bytes;
        }

        constexpr auto operator""_TiB(
            long double value
        ) -> unsigned long long int {
            return data_size(value, byte_multiple::TiB).bytes;
        }
    }
}
