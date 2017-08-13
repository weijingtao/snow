#pragma once

#include <string>
#include <boost/asio.hpp>

template <typename CODEC>
class client {
    using codec_t    = CODEC;
    using request_t  = CODEC::request_t;
    using response_t = CODEC::response_t;
    response_t request(const request_t& req) {
        const auto req_data = m_codec.encode(req);
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

    }

private:
    boost::asio::io_service&     m_ios;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::steady_timer    m_timer;
    codec_t                      m_codec;
    bool                         m_connected;
};