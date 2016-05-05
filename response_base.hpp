#ifndef _SNOW_RESPONSE_BASE_HPP
#define _SNOW_RESPONSE_BASE_HPP

namespace snow
{
    class response_base
    {
    public:
        ~response_base() {

        }

        virtual bool parse_from_array(const char* req_data, std::size_t req_len) = 0;

        virtual bool serialize_to_buffer(snow::buffer* rsp_buf) = 0;
    };
}

#endif //_SNOW_RESPONSE_BASE_HPP