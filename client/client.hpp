#pragma once

#include <string>
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include "../context.hpp"
#include "wait_group.hpp"
#include "../connection.hpp"
#include "wait_group.hpp"

namespace snow {

    class Client {
        template <typename Codec>
        class DoRequest;

    public:
        Client(boost::asio::io_service& ios, boost::asio::yield_context& yield)
                : m_ios{ios}
                , m_yield{yield} {

        }

        template <typename ... Codecs>
        bool request(Codecs& ...codecs, std::chrono::milliseconds timeout) {
            snow::WaitGroup wait_group{m_ios, m_yield};
            for(auto& codec : codecs) {
                std::make_shared<DoRequest<decltype(codec)>>(m_ios, wait_group, codec)->();
            }
            wait_group.wait(timeout);
            for(const auto& codec : codecs) {
                if(!codec) return false;
            }
            return true;
        }

    private:
        template <typename Codec>
        class DoRequest : public std::enable_shared_from_this<DoRequest> {
        public:
            DoRequest(boost::asio::io_service& ios, snow::WaitGroup& wait_group, Codec& codec)
                    : m_ios{ios}
                    , m_wait_group{wait_group}
                    , m_codec{codec}
                    , m_done_id{0} {

            }

            void operator()() {
                m_socket = std::move(get_tcp_socket());
                auto req_data = m_codec.encode();
                boost::asio::async_write(*m_socket,
                                         req_data,
                                         std::bind(&DoRequest<Codec>::write_finish_handler,
                                                   shared_from_this(),
                                                   std::placeholders::_1,
                                                   std::placeholders::_2));
                m_done_id = m_wait_group.add(std::bind(&DoRequest::timeout_handler, shared_from_this()));
            }

        private:
            using TcpSocket = boost::asio::ip::tcp::socket;
            using UdpSocket = boost::asio::ip::udp::socket;
            std::unique_ptr<TcpSocket> get_tcp_socket() {
                return nullptr;
            }

            std::unique_ptr<UdpSocket> get_udp_socket() {
                return nullptr;
            }

            void write_finish_handler(const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if(ec) {
                    m_codec.set_error_code(Codec::SEND_ERROR);
                    return;
                } else {
                    boost::asio::async_read(*m_socket,
                                            m_buf,
                                            std::bind(&DoRequest<Codec>::completion_condition,
                                                      shared_from_this(),
                                                      std::placeholders::_1,
                                                      std::placeholders::_2),
                                            std::bind(&DoRequest<Codec>::read_finish_handler,
                                                      shared_from_this(),
                                                      std::placeholders::_1,
                                                      std::placeholders::_2));
                }
            }

            std::size_t completion_condition(const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if(ec) {
                    m_codec.set_error_code(Codec::RECV_ERROR);
                    return 0;
                } else {
                    const int check_ret = m_codec.check(m_buf.c_str(), bytes_transferred);
                    if(check_ret > 0) {
                        return static_cast<std::size_t >(check_ret);
                    } else if(check_ret < 0) {
                        m_codec.set_error_code(Codec::Check_ERROR);
                        return 0;
                    } else {
                        return 0;
                    }
                }
            }

            void read_finish_handler(const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if(!ec) {
                    if(0 != m_codec.decode(m_buf.data(), bytes_transferred)) {
                        m_codec.set_error_code(Codec::DECODE_ERROR);
                    }
                }
                m_wait_group.done(m_done_id);
            }

            void timeout_handler() {
                m_socket->close();
            }


        private:
            boost::asio::io_service&   m_ios;
            snow::WaitGroup&           m_wait_group;
            Codec&                     m_codec;
            std::shared_ptr<TcpSocket> m_socket;
            std::string                m_buf;
            std::size_t                m_done_id;
        };

    private:
        boost::asio::io_service&     m_ios;
        boost::asio::yield_context&  m_yield;
    };
}