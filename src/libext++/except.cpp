#include <ext/except.h>

namespace ext {
    system_error::system_error(const std::string& message) :
        std::system_error(errno, std::generic_category(), message) {}
}
