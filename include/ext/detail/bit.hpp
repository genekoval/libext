#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>

namespace ext {
    /**
     * Reverses the bytes in the given integer value.
     */
    template <std::integral T>
    constexpr auto byteswap(T value) noexcept -> T {
        static_assert(
            std::has_unique_object_representations_v<T>,
            "T may not have padding bits"
        );

        auto value_representation =
            std::bit_cast<std::array<std::byte, sizeof(T)>>(value);

        std::ranges::reverse(value_representation);
        return std::bit_cast<T>(value_representation);
    }

    /**
     * Converts an integer from big endian to the host's endianness.
     *
     * On big endian, this is a no-op. On little endian, the bytes are swapped.
     */
    template <std::integral T>
    constexpr auto from_be(T t) noexcept -> T {
        if constexpr (std::endian::native == std::endian::big) return t;
        else return byteswap(t);
    }

    /**
     * Converts an integer from little endian to the host's endianness.
     *
     * On little endian, this is a no-op. On big endian, the bytes are swapped.
     */
    template <std::integral T>
    constexpr auto from_le(T t) noexcept -> T {
        if constexpr (std::endian::native == std::endian::little) return t;
        else return byteswap(t);
    }

    /**
     * Converts an integer to big endian from the host's endianness.
     *
     * On big endian, this is a no-op. On little endian, the bytes are swapped.
     */
    template <std::integral T>
    constexpr auto to_be(T t) noexcept -> T {
        if constexpr (std::endian::native == std::endian::big) return t;
        else return byteswap(t);
    }

    /**
     * Converts an integer to little endian from the host's endianness.
     *
     * On little endian, this is a no-op. On big endian, the bytes are swapped.
     */
    template <std::integral T>
    constexpr auto to_le(T t) noexcept -> T {
        if constexpr (std::endian::native == std::endian::little) return t;
        else return byteswap(t);
    }
}
