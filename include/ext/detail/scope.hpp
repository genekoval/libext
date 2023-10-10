#pragma once

#include <utility>

namespace ext {
    template <typename EF>
    class scope_exit {
        EF ef;
    public:
        scope_exit(EF&& ef) : ef(std::forward<EF>(ef)) {}

        ~scope_exit() { ef(); }
    };
}
