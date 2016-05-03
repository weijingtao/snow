#ifndef _SNOW_REQUEST_HPP
#define _SNOW_REQUEST_HPP

#include <cstdint>
#include <memory>
#include <chrono>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

namespace snow
{
    class request// : public std::enable_shared_from_this<request>
    {
    public:

        request()
            : m_time_out(0),
              m_dest_port(0)
        {

        }

        explicit request(const std::string& data)
            : m_time_out(0),
              m_data(std::move(data)),
              m_dest_port(0)
        {

        }

        request(const std::string& ip, uint16_t port, const std::string& data)
            : m_time_out(0),
              m_data(std::move(data)),
              m_dest_ip(ip),
              m_dest_port(port)
        {

        }

        void set_time_out(int time_out) { m_time_out = time_out; }

        int get_time_out() const { return m_time_out; }

        void set_ip(const std::string& ip) { m_dest_ip = ip; }

        const std::string& get_ip() const { return m_dest_ip; }

        void set_port(uint16_t port) { m_dest_port = port; }

        uint16_t get_port() const { return m_dest_port; }

        void set_data(const std::string& data) { m_data = std::move(data); }

        const std::string& get_data() const { return m_data; }


    private:
        int         m_time_out;
        std::string m_data;
        std::string m_dest_ip;
        uint16_t    m_dest_port;
    };
}


#endif //_SNOW_REQUEST_HPP