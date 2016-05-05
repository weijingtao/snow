#ifndef _SNOW_REQUEST_BASE_HPP
#define _SNOW_REQUEST_BASE_HPP

#include "buffer.hpp"

namespace snow
{
    class request_base
    {
    public:
        ~request_base() {

        }

        virtual bool parse_from_array(const char* req_data, std::size_t req_len) = 0;

        virtual bool serialize_to_buffer(snow::buffer* rsp_buf) = 0;
    };
}

#endif //_SNOW_REQUEST_BASE_HPP