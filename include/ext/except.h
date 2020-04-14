#pragma once

#include <string>
#include <system_error>

namespace ext {
    struct system_error : std::system_error {
        system_error(const std::string& message);
    };
}
