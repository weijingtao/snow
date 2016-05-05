#ifndef _SNOW_BUFFER_HPP
#define _SNOW_BUFFER_HPP

#include <cassert>
#include <memory>
#include "log.hpp"

namespace snow
{
    class buffer
    {
    public:
        static const std::size_t INIT_SIZE = 1024;
        buffer(std::size_t init_size = INIT_SIZE)
             : m_buffer(new char[init_size]),
               m_size(init_size),
               m_read_index(0),
               m_write_index(0) {
            SNOW_LOG_TRACE << "buffer construct" << std::endl;
        }

        //TODO
        buffer(buffer&& rhs)
            : m_size(rhs.m_size),
              m_read_index(rhs.m_read_index),
              m_write_index(rhs.m_write_index) {
            m_buffer.swap(rhs.m_buffer);
            rhs.m_size        = 0;
            rhs.m_read_index  = 0;
            rhs.m_write_index = 0;
            SNOW_LOG_TRACE << "buffer move construct" << std::endl;
        }

        ~buffer() {
            SNOW_LOG_TRACE << "buffer destruct" << std::endl;
        }

        std::size_t readable_bytes() const {
            return m_write_index - m_read_index;
        }

        std::size_t writeable_bytes() const {
            return m_size - m_write_index;
        }

        char* read_index() {
            return m_buffer.get() + m_read_index;
        }

        const char* read_index() const {
            return m_buffer.get() + m_read_index;
        }

        char* write_index() {
            return m_buffer.get() + m_write_index;
        }

        const char* write_index() const {
            return m_buffer.get() + m_write_index;
        }

        void increase_read_index(std::size_t size) {
            m_read_index += size;
        }

        void increase_write_index(std::size_t size) {
            m_write_index += size;
        }

        void append(const char * data, std::size_t len) {
            ensure_writeable_bytes(len);
            std::copy(data, data + len, read_index());
            increase_write_index(len);
        }

        void ensure_writeable_bytes(std::size_t size) {
            if(writeable_bytes() >= size) {
                return;
            }
            if (m_size - readable_bytes() < size) {
                std::unique_ptr<char[]> new_buffer(new char[m_write_index + size]);
                std::copy(read_index(), write_index(), new_buffer.get());
                m_buffer.swap(new_buffer);
            }
            std::size_t readable = readable_bytes();
            m_read_index         = 0;
            m_write_index        = m_read_index + readable;
            assert(writeable_bytes() >= size);
        }

    private:
        std::unique_ptr<char[]> m_buffer;
        std::size_t             m_size;
        std::size_t             m_read_index;
        std::size_t             m_write_index;
    };
}

#endif //_SNOW_BUFFER_HPP