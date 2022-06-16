#pragma once

#include <concepts>
#include <cmath>

namespace ext {
    /**
     * Returns the number of digits in n.
     */
    template <std::integral N>
    constexpr auto digits(N n) -> std::size_t {
        // ln(0) is undefined
        if (n == 0) return 1;

        // Run computations on positive numbers.
        const auto abs = n < 0 ? n * -1 : n;

        return static_cast<std::size_t>(std::log(abs) / std::log(10)) + 1;
    }
}
