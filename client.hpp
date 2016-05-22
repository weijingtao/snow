#ifndef _SNOW_CLIENT_HPP
#define _SNOW_CLIENT_HPP

#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include "session_base.hpp"
#include "buffer.hpp"

namespace snow
{
    template <typename RequestType, typename ResponseType, typename PkgCheckType>
    class client
    {
    public:
        typedef RequestType                                            request_type;
        typedef ResponseType                                           response_type;
        typedef PkgCheckType                                           pkg_check_type;
        typedef std::pair<request_type, response_type, pkg_check_type> request_item;

        explicit client(session_base& session)
             : m_session(session.shared_from_this()),
               m_strand(m_session->get_strand()),
               m_timer(m_session->get_strand().get_io_service()) {

        }

        bool request(const request_type& req, response_type* rsp) {
            auto self(shared_from_this());
            uint32_t dest_ip = inet_addr(req.get_ip().c_str());
            dest_ip = ntohl(dest_ip);
            boost::asio::ip::address_v4 addr(dest_ip);
            boost::asio::ip::tcp::endpoint dest_addr(addr, req.get_port());
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_strand.get_io_service());
            socket->connect(dest_addr);
            bool result = false;

            boost::asio::spawn(m_strand, [this, self, &socket, &req, &rsp](boost::asio::yield_context yield) {
                try {
                    m_timer.expires_from_now(std::chrono::seconds(std::min(req.get_time_out(), m_session->get_time_left())));
                    boost::asio::async_write(*socket, boost::asio::buffer("hello"), yield);
                    std::cout << "send success" << std::endl;
                    buffer recv_buffer;
                    std::size_t n_read  = 0;
                    std::size_t pkg_len = 0;
                    do {
                        recv_buffer.ensure_writeable_bytes(256);
                        n_read = socket->async_read_some(boost::asio::buffer(recv_buffer.write_index(), recv_buffer.writeable_bytes()), yield);
                        if(n_read > 0) {
                            recv_buffer.increase_write_index(n_read);
                        } else {
                            socket->close();
                            m_timer.cancel();
                        }
                        pkg_len = m_pkg_checker(recv_buffer.read_index(), recv_buffer.readable_bytes());
                    } while(0 == pkg_len);
                    if(pkg_len > 0) {
                        result = true;
                        resume();
                    } else {
                        socket->close();
                        m_timer.cancel();
                        resume();
                    }
                } catch (std::exception& e) {
                    socket->close();
                    m_timer.cancel();
                    resume();
                }
            });

            boost::asio::spawn(m_strand, [this, self, &socket, &rsp](boost::asio::yield_context yield) {
                while (socket->is_open()) {
                    boost::system::error_code ignored_ec;
                    m_timer.async_wait(yield[ignored_ec]);
                    if (m_timer.expires_from_now() <= std::chrono::seconds(0)) {
                        socket->close();
                        resume();
                    }
                }
            });

            m_session->yield();
            return result;
        }

        bool batch_request(std::vector<request_item>& reqs) {
            return true;
        }

    private:
        void resume() {
            static bool flag = false;
            if(!flag) {
                m_session->resume();
                flag = true;
            }
        }

    private:
        std::shared_ptr<session_base> m_session;
        boost::asio::strand&          m_strand;
        boost::asio::steady_timer     m_timer;
        pkg_check_type                m_pkg_checker;
    };
}

#endif //_SNOW_CLIENT_HPP