#include <nova/ext/string.h>

#include <stdexcept>

constexpr auto environment_variable_regex = "\\$([a-zA-Z_]+[a-zA-Z0-9_]*)";

namespace nova::ext::string {
    auto expand_env(const std::string& str) -> std::string {
        return replace<std::string>(
            str,
            std::regex(environment_variable_regex),
            [&str](const std::smatch& match) -> std::string {
                const auto s = match[1].str();

                if (auto env = std::getenv(s.c_str())) return env;

                throw std::invalid_argument(
                    "undefined environment variable " QUOTE(s) " in sequence: "
                        + str
                );
            }
        );
    }
}