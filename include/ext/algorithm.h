#pragma once

#include <algorithm>

namespace ext {
    template <typename Container, typename T = typename Container::value_type>
    auto contains(const Container& container, const T& value) -> bool {
        return std::find(container.begin(), container.end(), value) !=
               container.end();
    }
}
