#pragma once

#include <string>
#include <system_error>

namespace nova::ext::except {
    struct system_error : std::system_error {
        system_error(const std::string& message) :
            std::system_error(errno, std::generic_category(), message)
        {}
    };
}
