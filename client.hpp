#ifndef _SNOW_CLIENT_HPP
#define _SNOW_CLIENT_HPP

#include <memory>
#include <utility>
#include <functional>
#include <chrono>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include "session_base.hpp"
#include "buffer.hpp"
#include "log.hpp"
#include "request_base.hpp"
#include "response_base.hpp"

namespace snow
{
    class client : public std::enable_shared_from_this<client>
    {
    public:
        typedef std::shared_ptr<boost::asio::ip::tcp::socket>           tcp_socket_type;
        typedef std::shared_ptr<boost::asio::ip::udp::socket>           udp_socket_type;
        typedef std::reference_wrapper<bool>                            result_type;
        typedef std::function<bool(buffer*)>                            request_serializer_type;
        typedef std::function<bool(const char*, std::size_t)>           response_parser_type;
        typedef std::function<std::size_t(const char*, std::size_t)>    pkg_spliter_type;
        typedef std::tuple<result_type, tcp_socket_type, request_serializer_type, response_parser_type, pkg_spliter_type> request_type;

        explicit client(session_base& session)
             : m_session(session.shared_from_this()),
               m_strand(m_session->get_strand()),
               m_timer(m_session->get_strand().get_io_service()),
               m_complete_request_count(0) {

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

        void batch_request(std::vector<request_type>& requests) {
            if(requests.empty()) {
                return;
            }
            m_requests = requests;
            auto self(shared_from_this());
            m_timer.expires_from_now(std::chrono::seconds(std::min(req.get_time_out(), m_session->get_time_left())));
            for(auto& request : m_requests) {
                boost::asio::spawn(m_strand, [this, self, &request](boost::asio::yield_context yield) {
                    try {
                        {
                            buffer send_buffer;
                            std::get<2>(request)(&send_buffer);
                            boost::asio::async_write(*std::get<1>(request), boost::asio::buffer(send_buffer.read_index(), send_buffer.readable_bytes()), yield);
                            SNOW_LOG_TRACE << "send " << send_buffer.readable_bytes() << " bytes" << std::endl;
                        }
                        buffer recv_buffer;
                        std::size_t n_read  = 0;
                        std::size_t pkg_len = 0;
                        do {
                            recv_buffer.ensure_writeable_bytes(256);
                            n_read = socket->async_read_some(boost::asio::buffer(recv_buffer.write_index(), recv_buffer.writeable_bytes()), yield);
                            SNOW_LOG_TRACE << "recv " << n_read << " bytes" << std::endl;
                            if(n_read > 0) {
                                recv_buffer.increase_write_index(n_read);
                            } else {
                                break;
                            }
                            pkg_len = std::get<4>(request)(recv_buffer.read_index(), recv_buffer.readable_bytes());
                        } while(0 == pkg_len);
                        SNOW_LOG_TRACE << "pacgage len " << pkg_len << " bytes" << std::endl;
                        if(pkg_len > 0) {
                            std::get<0>(request) = true;
                            std::get<3>(request)(recv_buffer.read_index(), recv_buffer.readable_bytes());
                        } else {
                            std::get<1>(request)->close();
                        }
                    } catch (std::exception& e) {
                        std::get<1>(request)->close();
                    }
                    complete(1);
                });
            }

            boost::asio::spawn(m_strand, [this, self](boost::asio::yield_context yield) {
                boost::system::error_code ignored_ec;
                m_timer.async_wait(yield[ignored_ec]);
                if (m_timer.expires_from_now() <= std::chrono::seconds(0)) {
                    for(auto& request : m_requests) {
                        if(std::get<1>(request)->is_open()) {
                            std::get<1>(request)->close();
                        }
                    }
                    //TODO need?
                }
            });
            m_session->yield();
        }

    protected:
        //子类可重载这两个函数，实现连接池等功能
        virtual std::shared_ptr<boost::asio::ip::tcp::socket> get_tcp_socket(const std::string& address) const {
            uint32_t dest_ip = inet_addr(req.get_ip().c_str());
            dest_ip = ntohl(dest_ip);
            boost::asio::ip::address_v4 addr(dest_ip);
            boost::asio::ip::tcp::endpoint dest_addr(addr, req.get_port());
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_strand.get_io_service());
            socket->connect(dest_addr);
            return socket;
        }

        virtual std::shared_ptr<boost::asio::ip::udp::socket> get_udp_socket(const std::string& address) const {
            return std::make_shared<boost::asio::ip::udp::socket>();
        }


    private:
        void complete(std::size_t count) {
            m_complete_request_count += count;
            if(m_complete_request_count >= m_requests.size()) {
                resume();
            }
        }

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
        std::vector<request_type>     m_requests;
        std::size_t                   m_complete_request_count;
        std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> m_sockets;
    };
}

#endif //_SNOW_CLIENT_HPP