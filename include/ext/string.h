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
    auto expand_env(std::string_view sequence) -> std::string;

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
    template <typename Callable>
    auto replace(
        std::string_view sequence,
        const std::regex& pattern,
        Callable&& replacement
    ) -> std::string {
        using iterator = std::regex_iterator<std::string_view::iterator>;

        auto it = iterator(sequence.begin(), sequence.end(), pattern);
        auto end = iterator();

        if (it == end) return std::string(sequence);

        auto os = std::ostringstream();

        while (it != end) {
            const auto& match = *it;

            os << match.prefix();
            os << replacement(match);
            if (++it == end) os << match.suffix();
        }

        return os.str();
    }

    class string_range {
        const std::string delimiter;
        const std::string_view sequence;
    public:
        string_range(std::string_view sequence, std::string_view delimiter) :
            delimiter(delimiter),
            sequence(sequence)
        {}

        class iterator {
        public:
            using difference_type = std::string_view::size_type;
            using value_type = std::string_view;
            using pointer = const value_type*;
            using reference = value_type;
            using iterator_category = std::forward_iterator_tag;
        private:
            difference_type first;
            difference_type last;
            const string_range* range;

            auto advance() -> void {
                if (last == value_type::npos) {
                    first = value_type::npos;
                    return;
                }

                first = last + range->delimiter.size();
                last = range->sequence.find(range->delimiter, first);
            }
        public:
            iterator() : first(value_type::npos), last(value_type::npos) {}

            iterator(const string_range* range) :
                first(0),
                last(range->sequence.find(range->delimiter)),
                range(range)
            {}

            auto operator++() -> iterator& {
                advance();
                return *this;
            }

            auto operator++(int) -> iterator {
                auto tmp = *this;
                operator++();
                return tmp;
            }

            auto operator==(const iterator& other) const -> bool {
                return first == other.first && last == other.last;
            }

            auto operator!=(const iterator& other) const -> bool {
                return !(*this == other);
            }

            auto operator*() -> reference {
                return range->sequence.substr(first, last - first);
            }
        };

        auto begin() -> iterator { return iterator(this); }

        auto end() -> iterator { return iterator(); }
    };

    auto split(
        std::string_view sequence,
        std::string_view delimiter
    ) -> std::vector<std::string_view>;

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
