#pragma once

#include <string>
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include "wait_group.hpp"
#include "../connection.hpp"
#include "wait_group.hpp"
#include "codec.hpp"
#include "utils/fixed_size_buffer.hpp"
#include "log.hpp"

namespace snow {

    class Client {
        template <typename Codec>
        class DoRequest;

    public:
        Client(boost::asio::io_service& ios, boost::asio::yield_context& yield)
                : m_ios{ios}
                , m_yield{yield} {
            SNOW_LOG_TRACE << __func__ << std::endl;
        }

        template <typename... Codecs>
        bool request(Codecs& ...codecs, std::chrono::milliseconds timeout) {
            snow::WaitGroup wait_group{m_ios, m_yield};
            _request(m_ios, wait_group, codecs...);
            wait_group.wait(timeout);
            /*for(const auto& codec : codecs...) {
                if(!codec) return false;
            }*/
            return true;
        }

        template <typename Codec>
        void _request(boost::asio::io_service& ios, WaitGroup& wait_group, Codec& codec) {
            std::make_shared<DoRequest<decltype(codec)>>(m_ios, wait_group, codec)->start();
        }

    private:
        template <typename Codec>
        class DoRequest : public std::enable_shared_from_this<DoRequest<Codec>> {
        public:
            DoRequest(boost::asio::io_service& ios, snow::WaitGroup& wait_group, Codec& codec)
                    : m_ios{ios}
                    , m_wait_group{wait_group}
                    , m_codec{codec}
                    , m_done_id{0} {

            }

            void start() {
                m_socket = std::move(get_tcp_socket());
                FixedSizeBuffer<4096> encode_buffer;
                if(!m_codec.encode(encode_buffer)) {
                    m_codec.set_error_code(ENCODE_ERROR);
                    return;
                }
                SNOW_LOG_TRACE << "readable bytes " << encode_buffer.readable_bytes() << std::endl;
                boost::asio::async_write(*m_socket,
                                         boost::asio::buffer(encode_buffer.read_index(), encode_buffer.readable_bytes()),
                                         std::bind(&DoRequest<Codec>::write_finish_handler,
                                                   this->shared_from_this(),
                                                   std::placeholders::_1,
                                                   std::placeholders::_2));
                m_done_id = m_wait_group.add(std::bind(&DoRequest::timeout_handler, this->shared_from_this()));
            }

        private:
            using TcpSocket = boost::asio::ip::tcp::socket;
            using UdpSocket = boost::asio::ip::udp::socket;
            std::unique_ptr<TcpSocket> get_tcp_socket() {
                boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 10000);
                std::unique_ptr<TcpSocket> socket{new TcpSocket(m_ios)};
                socket->connect(endpoint);
                return std::move(socket);
            }

            std::unique_ptr<UdpSocket> get_udp_socket() {
                return nullptr;
            }

            void write_finish_handler(const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if(ec) {
                    m_codec.set_error_code(SEND_ERROR);
                    return;
                } else {
                    SNOW_LOG_TRACE << "write success" << std::endl;
                    boost::asio::async_read(*m_socket,
                                            boost::asio::buffer(m_buf.write_index(), m_buf.writeable_bytes()),
                                            std::bind(&DoRequest<Codec>::completion_condition,
                                                      this->shared_from_this(),
                                                      std::placeholders::_1,
                                                      std::placeholders::_2),
                                            std::bind(&DoRequest<Codec>::read_finish_handler,
                                                      this->shared_from_this(),
                                                      std::placeholders::_1,
                                                      std::placeholders::_2));
                }
            }

            std::size_t completion_condition(const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if(ec) {
                    m_codec.set_error_code(RECV_ERROR);
                    return 0;
                } else {
                    const int check_ret = m_codec.check(m_buf.read_index(), bytes_transferred);
                    SNOW_LOG_TRACE << "check ret " << check_ret << std::endl;
                    if(check_ret > 0) {
                        return 0;
                    } else if(check_ret == 0) {
                        return m_buf.writeable_bytes();
                    } else {
                        m_codec.set_error_code(Check_ERROR);
                        return 0;
                    }
                }
            }

            void read_finish_handler(const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if(!ec) {
                    m_buf.increase_write_index(bytes_transferred);
                    if(!m_codec.decode(m_buf)) {
                        m_codec.set_error_code(DECODE_ERROR);
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
            FixedSizeBuffer<4096>      m_buf;
            std::size_t                m_done_id;
        };

    private:
        boost::asio::io_service&     m_ios;
        boost::asio::yield_context&  m_yield;
    };
}