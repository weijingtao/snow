#ifndef _SNOW_RESPONSE_HPP
#define _SNOW_RESPONSE_HPP

#include <cstdint>
#include <string>

namespace snow
{
    class request;
    class response
    {
    public:
        using data_type = std::string;
        explicit response()
            : m_result(0)
        {

        }

        response(int result)
            : m_result(result)
        {

        }

        response(int result, const data_type & data)
            : m_result(0),
              m_data(std::move(data))
        {

        }

        void set_result(int result) { m_result = result; }

        int get_result() const { return m_result; }

        void set_data(const data_type& data) { m_data = std::move(data); }

        const data_type& get_data() const { return m_data; }

        data_type& get_data() { return m_data; }

    private:
        int            m_result;
        data_type      m_data;
    };
}

#endif //_SNOW_RESPONSE_HPP