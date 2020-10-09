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
                    "undefined environment variable " QUOTE(s) " in sequence: "
                        + std::string(string)
                );
            }
        );
    }

    auto split(
        std::string_view sequence,
        std::string_view delimiter
    ) -> std::vector<std::string_view> {
        auto range = ext::string_range(sequence, delimiter);
        return std::vector<std::string_view>(range.begin(), range.end());
    }

    auto trim(std::string string) -> std::string {
        return trim_left(trim_right(string));
    }

    auto trim_left(std::string string) -> std::string {
        string.erase(
            string.begin(),
            std::find_if(string.begin(), string.end(), [](unsigned char c) {
                return !std::isspace(c);
            })
        );
        return string;
    }

    auto trim_right(std::string string) -> std::string {
        if (!string.size()) return string;

        auto it = --string.end();
        while (std::isspace(*it)) it--;

        string.erase(++it, string.end());
        return string;
    }

    auto quote(std::string_view text) -> std::string {
        auto os = std::stringstream();
        os << "‘" << text << "‘";
        return os.str();
    }
}
