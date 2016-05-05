#ifndef _SNOW_CLIENT_HPP
#define _SNOW_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include "session_base.hpp"

namespace snow
{
    template <typename RequestType, typename ResponseType, typename PkgCheckType>
    class client
    {
    public:
        typedef RequestType  request_type;
        typedef ResponseType response_type;
        typedef PkgCheckType pkg_check_type;

        explicit client(session_base& session)
             : m_session(session),
               m_timer(m_session.get_strand().get_io_service()) {

        }

        response_type do_request(const request_type& req, boost::asio::yield_context& yield){
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

    private:
        session_base&              m_session;
        boost::asio::steady_timer  m_timer;
    };
}

#endif //_SNOW_CLIENT_HPP