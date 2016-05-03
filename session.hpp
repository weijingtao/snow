#ifndef _SNOW_SESSION_HPP
#define _SNOW_SESSION_HPP

#include <cstdint>
#include <memory>
#include <array>
#include <functional>
#include <string>
#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include "request.hpp"
#include "response.hpp"

namespace snow
{
    class session : public std::enable_shared_from_this<session>
    {
    public:
        explicit session(boost::asio::io_service& ios)
                : m_strand(ios),
                  m_timer(ios) {

        }

        virtual void process(const request& req, response* rsp, boost::asio::yield_context yield) = 0;

        virtual response do_request(const request& req, boost::asio::yield_context& yield){
            auto self(shared_from_this());
            uint32_t dest_ip = inet_addr(req.get_ip().c_str());
            dest_ip = ntohl(dest_ip);
            boost::asio::ip::address_v4 addr(dest_ip);
            boost::asio::ip::tcp::endpoint dest_addr(addr, req.get_port());
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_strand.get_io_service());
            socket->connect(dest_addr);
            response rsp(req);

            boost::asio::spawn(m_strand, [this, self, &socket, &req, &rsp](boost::asio::yield_context yield) {
                try {
                    m_timer.expires_from_now(std::chrono::seconds(std::min(req.get_time_out(), m_time_left)));
                    boost::asio::async_write(*socket, boost::asio::buffer("hello"), yield);
                    std::cout << "send success" << std::endl;

//                    socket->async_read_some(boost::asio::buffer(rsp.get_data().data(), rsp.get_data().size()), yield);
                    std::cout << "recv success" << std::endl;
                    std::cout << rsp.get_data().data() << std::endl;
                } catch (std::exception& e) {
                    socket->close();
                    m_timer.cancel();
                }
            });

            boost::asio::spawn(m_strand, [this, self, &socket, &rsp](boost::asio::yield_context yield) {
                while (socket->is_open()) {
                    boost::system::error_code ignored_ec;
                    m_timer.async_wait(yield[ignored_ec]);
                    if (m_timer.expires_from_now() <= std::chrono::seconds(0)) {
                        socket->close();
                        rsp.set_result(-1);
                    }
                }
            });

            return std::move(rsp);
        }


    protected:
        boost::asio::io_service::strand               m_strand;
        boost::asio::steady_timer                     m_timer;
        int       m_time_left;
    };
}



#endif //_SNOW_SESSION_HPP