#pragma once

#include <functional>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace nova::ext::string {
    using string_map = std::unordered_map<std::string,std::string>;

    template <
        typename Container,
        typename Value = typename Container::value_type
    >
    std::string join(const Container& elements, const char* delimiter) {
        std::ostringstream os;
        auto begin = elements.begin();
        auto end = elements.end();

        if (begin != end) {
            std::ostream_iterator<Value> oit(os, delimiter);
            std::copy(begin, std::prev(end), oit);

            begin = std::prev(end);
            if (begin != end) os << *begin;
        }

        return os.str();
    }

    template <typename T>
    std::string replace(
        const std::string& sequence,
        const std::regex& re,
        const std::function<T(const std::smatch&)>& replacement
    ) {
        auto it = std::sregex_iterator(sequence.begin(), sequence.end(), re);
        auto end = std::sregex_iterator();

        if (it == end) return sequence;

        std::ostringstream os;

        while (it != end) {
            std::smatch match = *it;

            os << match.prefix();
            os << replacement(match);
            if (++it == end) os << match.suffix();
        }

        return os.str();
    }

    template <typename Container>
    void split(
        Container& result,
        const typename Container::value_type& s,
        const typename Container::value_type& sequence
    ) {
        result.clear();

        size_t size = sequence.size();
        size_t current = 0;
        size_t next = 0;

        do {
            next = s.find(sequence, current);

            auto field = s.substr(current, next - current);
            if (!field.empty()) result.push_back(field);
        } while (
            next != Container::value_type::npos &&
            (current = next + size) <= s.size()
        );
    }
}
