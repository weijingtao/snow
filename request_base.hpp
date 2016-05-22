#ifndef _SNOW_REQUEST_BASE_HPP
#define _SNOW_REQUEST_BASE_HPP

#include <string>
#include "buffer.hpp"

namespace snow
{
    class request_base
    {
    public:
        ~request_base() {

        }

        virtual bool parse_from_array(const char* data, std::size_t len) = 0;

        virtual bool serialize_to_buffer(snow::buffer* rsp_buf) = 0;

        void set_address(const std::string& address) {
            m_address = address;
        }

    private:
        std::string m_address;
    };
}

#endif //_SNOW_REQUEST_BASE_HPP