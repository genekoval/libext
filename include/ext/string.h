#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

/**
 *  Surrounds the given string with special quotation marks.
 */
#define QUOTE(string) "‘" + string + "‘"

/**
 * Surrounds the given string view with special quotation marks.
 */
#define QUOTE_VIEW(string_view) QUOTE(std::string(string_view))

namespace ext {
    using string_map = std::unordered_map<std::string,std::string>;

    /**
     * Replaces any environment variables in the specified string with their
     * expanded values.
     *
     * @param sequence The string to search for environment variables.
     * @return A new string with all environment variables expanded.
     * @throw std::invalid_argument An undefined environment variable was found.
     */
    auto expand_env(const std::string& sequence) -> std::string;

    /**
     * Returns a new string composed of copies of the Container elements joined
     * together with a copy of the specified delimiter.
     *
     * @param elements The elements to join together.
     * @param delimiter The delimiter that separates each element.
     * @return A new string that is composed of the elements separated by the
     * delimiter.
     */
    template <
        typename Container,
        typename Value = typename Container::value_type
    >
    auto join(const Container& elements, const char* delimiter) -> std::string {
        auto os = std::ostringstream();
        auto begin = elements.begin();
        auto end = elements.end();

        if (begin != end) {
            auto it = std::ostream_iterator<Value>(os, delimiter);
            std::copy(begin, std::prev(end), it);

            begin = std::prev(end);
            if (begin != end) os << *begin;
        }

        return os.str();
    }

    /**
     * Returns a new string with some or all matches of a pattern replaced by a
     * replacement.
     *
     * Matches found in the original string are passed to a
     * replacement function. The result of this function call is inserted
     * in the match's place within the returned string.
     *
     * @param sequence The original string to search for matches.
     * @param pattern The regular expression to use for finding matches.
     * @param replacement A function to be invoked to be used to replace the
     * matches to the given pattern.
     * @return A new string, with some or all matches of a pattern replaced by a
     * replacement.
     */
    template <typename T>
    auto replace(
        const std::string& sequence,
        const std::regex& pattern,
        const std::function<T(const std::smatch&)>& replacement
    ) -> std::string {
        auto it = std::sregex_iterator(
            sequence.begin(),
            sequence.end(),
            pattern
        );

        auto end = std::sregex_iterator();

        if (it == end) return sequence;

        auto os = std::ostringstream();

        while (it != end) {
            auto match = *it;

            os << match.prefix();
            os << replacement(match);
            if (++it == end) os << match.suffix();
        }

        return os.str();
    }

    // TODO Document this method. Maybe get a better parameter name than 's'.
    template <
        typename Container,
        typename Value = typename Container::value_type
    >
    auto split(
        Container& result,
        const Value& s,
        const Value& sequence
    ) -> void {
        result.clear();

        size_t size = sequence.size();
        size_t current = 0;
        size_t next = 0;

        do {
            next = s.find(sequence, current);

            auto field = s.substr(current, next - current);
            if (!field.empty()) result.push_back(field);
        } while (
            next != Value::npos &&
            (current = next + size) <= s.size()
        );
    }

    /**
     * Returns a new string with all leading and trailing whitespace removed
     * from the given string.
     *
     * @param string The string from which to remove whitespace.
     * @return A new string with all leading and trailing whitespace removed.
     */
    auto trim(std::string string) -> std::string;

    /**
     * Returns a new string with all leading whitespace removed from the given
     * string.
     *
     * @param str The string from which to remove whitespace.
     * @return A new string with all leading whitespace removed.
     */
    auto trim_left(std::string string) -> std::string;

    /**
     * Returns a new string with all trailing whitespace removed from the given
     * string.
     *
     * @param str The string from which to remove whitespace.
     * @return A new string with all trailing whitespace removed.
     */
    auto trim_right(std::string string) -> std::string;
}
