#pragma once

#include <cassert>
#include <memory>
#include <array>
#include "log/log.hpp"

namespace snow {
template<std::size_t MaxSize>
class FixedSizeBuffer {
public:
    FixedSizeBuffer()
            : m_read_index(0),
              m_write_index(0) {
        SNOW_LOG_TRACE("buffer construct");
    }

    ~FixedSizeBuffer() = default;

    std::size_t readable_bytes() const {
        return m_write_index - m_read_index;
    }

    std::size_t writeable_bytes() const {
        return m_buffer.size() - m_write_index;
    }

    char *read_index() {
        return m_buffer.data() + m_read_index;
    }

    const char *read_index() const {
        return m_buffer.data() + m_read_index;
    }

    char *write_index() {
        return m_buffer.data() + m_write_index;
    }

    const char *write_index() const {
        return m_buffer.data() + m_write_index;
    }

    void increase_read_index(std::size_t size) {
        m_read_index += size;
    }

    void increase_write_index(std::size_t size) {
        m_write_index += size;
    }

    void append(const char *data, std::size_t len) {
        std::copy(data, data + len, write_index());
        increase_write_index(len);
    }

    void adjuest() {
        if (m_write_index == m_read_index) {
            m_write_index = m_read_index = 0;
        }
        std::move(m_buffer.begin() + m_read_index,
                  m_buffer.begin() + m_write_index,
                  m_buffer.begin());
        std::size_t old_read_index = m_read_index;
        m_read_index = 0;
        m_write_index -= old_read_index;
    }

private:
    std::array<char, MaxSize> m_buffer;
    std::size_t m_read_index;
    std::size_t m_write_index;
};
}