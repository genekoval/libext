#include <ext/string.h>

#include <stdexcept>

namespace ext {
    constexpr auto environment_variable_regex = "\\$([a-zA-Z_]+[a-zA-Z0-9_]*)";

    auto expand_env(std::string_view string) -> std::string {
        return replace(
            string,
            std::regex(environment_variable_regex),
            [&string](const std::cmatch& match) -> std::string {
                const auto s = match[1].str();

                if (auto env = std::getenv(s.c_str())) return env;

                throw std::invalid_argument(
                    "undefined environment variable " QUOTE(s
                    ) " in sequence: " +
                    std::string(string)
                );
            }
        );
    }

    auto split(std::string_view sequence, std::string_view delimiter)
        -> std::vector<std::string_view> {
        auto range = ext::string_range(sequence, delimiter);
        return std::vector<std::string_view>(range.begin(), range.end());
    }

    auto trim(std::string_view string) -> std::string_view {
        return trim_end(trim_start(string));
    }

    auto trim_start(std::string_view string) -> std::string_view {
        const auto* begin = string.begin();

        const auto* result =
            std::find_if(begin, string.end(), [](unsigned char c) -> bool {
                return !std::isspace(c);
            });

        string.remove_prefix(std::distance(begin, result));
        return string;
    }

    auto trim_end(std::string_view string) -> std::string_view {
        const auto begin = string.rbegin();

        const auto result =
            std::find_if(begin, string.rend(), [](unsigned char c) -> bool {
                return !std::isspace(c);
            });

        string.remove_suffix(std::distance(begin, result));
        return string;
    }

    auto quote(std::string_view text) -> std::string {
        auto os = std::stringstream();
        os << "‘" << text << "‘";
        return os.str();
    }
}
