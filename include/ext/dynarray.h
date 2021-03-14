#pragma once

#include <cstring>
#include <memory>
#include <span>

namespace ext {
    template <typename T>
    class dynarray {
        std::unique_ptr<T[]> m_data;
        std::size_t m_size = 0;
        std::span<T> m_span;
    public:
        dynarray() = default;

        dynarray(std::size_t size) :
            m_data(std::make_unique<T[]>(size)),
            m_size(size),
            m_span(m_data.get(), m_size)
        {}

        auto copy(
            const T* src,
            std::size_t count,
            std::size_t offset = 0
        ) const -> void {
            std::memcpy(&m_span[offset], src, count);
        }

        auto data() const -> T* {
            return m_data.get();
        }

        auto release() -> T* {
            m_size = 0;
            m_span = std::span<T>();
            return m_data.release();
        }

        auto size() const -> std::size_t {
            return m_size;
        }

        auto span() const -> std::span<T> {
            return m_span;
        }
    };
}
