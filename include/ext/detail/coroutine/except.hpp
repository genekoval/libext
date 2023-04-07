#pragma once

#include <stdexcept>

namespace ext {
    struct broken_promise : std::logic_error {
        broken_promise() : std::logic_error("broken promise") {}
    };

    struct no_result : std::exception {
        auto what() const noexcept -> const char* override {
            return "result not set";
        }
    };
}
