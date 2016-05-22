#ifndef _SNOW_RESPONSE_BASE_HPP
#define _SNOW_RESPONSE_BASE_HPP

namespace snow
{
    class response_base
    {
    public:
        response_base()
            : m_result(false) {

        }

        virtual ~response_base() {

        }

        virtual bool parse_from_array(const char* data, std::size_t len) = 0;

        virtual bool serialize_to_buffer(snow::buffer* rsp_buf) = 0;

        operator bool() {
            return m_result;
        }

    private:
        friend class client;
        bool m_result;
    };
}

#endif //_SNOW_RESPONSE_BASE_HPP