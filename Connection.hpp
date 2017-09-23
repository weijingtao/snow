#pragma once

#include <memory>
#include <functional>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include "utils/fixed_size_buffer.hpp"
#include "utils/type.hpp"
#include "utils/status.hpp"
#include "log/log.hpp"
#include "spdlog/fmt/bundled/ostream.h"

namespace snow {
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::ip::tcp::socket &socket,
               utils::request_dispatch_t request_dispatcher,
               utils::pkg_split_t pkg_spliter,
               std::size_t time_out)
            : m_socket(std::move(socket)),
              m_timer(m_socket.get_io_service()),
              m_request_dispatcher(std::move(request_dispatcher)),
              m_pkg_spliter(std::move(pkg_spliter)),
              m_time_out(time_out) {
        SNOW_LOG_TRACE("socket fd {}, {}<->{}, timeout {}ms",
                       m_socket.native(),
                       m_socket.local_endpoint(),
                       m_socket.remote_endpoint(),
                       m_time_out.count());
    }

    ~Connection() {
        if (m_socket.is_open()) {
            m_socket.close();
        }
        m_timer.cancel();
        SNOW_LOG_TRACE("socket fd {}, status {}", m_socket.native(), m_status);
    }

    void start() {
        SNOW_LOG_TRACE("socket fd {} start read, timeout {}ms", m_socket.native(), m_time_out.count());
        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_recv_buffer.write_index(), m_recv_buffer.writeable_bytes()),
                                std::bind(&Connection::completion_condition,
                                          this->shared_from_this(),
                                          std::placeholders::_1,
                                          std::placeholders::_2),
                                std::bind(&Connection::read_finish_handler,
                                          this->shared_from_this(),
                                          std::placeholders::_1,
                                          std::placeholders::_2));
        m_timer.expires_from_now(m_time_out);
        m_timer.async_wait(std::bind(&Connection::timeout_handler, this->shared_from_this()));
    }

private:
    int send(const char *const data, std::size_t size) {
        if (0 == size) {
            SNOW_LOG_WARN("send data is nil");
            return 0;
        }
        if (m_send_buffer.writeable_bytes() < size) {
            SNOW_LOG_WARN("send buffer do not has enough space, vailable {}, need {}",
                          m_send_buffer.writeable_bytes(),
                          size);
            return 0;
        }
        restart_timer();
        bool need_start_write = m_send_buffer.readable_bytes() == 0;
        m_send_buffer.append(data, size);
        if (need_start_write) {
            SNOW_LOG_TRACE("start async write, size {}", size);
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_send_buffer.read_index(),
                                                         m_send_buffer.readable_bytes()),
                                     std::bind(&Connection::write_finish_handler,
                                               this->shared_from_this(),
                                               std::placeholders::_1,
                                               std::placeholders::_2));
        }
        return 0;
    }

    void write_finish_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) {
        if (ec || !m_socket.is_open()) {
            m_timer.cancel();
            m_socket.cancel();
            m_status = utils::Status::WriteError();
            return;
        }
        SNOW_LOG_TRACE("write success");
        m_send_buffer.increase_read_index(bytes_transferred);
        if (m_send_buffer.readable_bytes() > 0) {
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_send_buffer.read_index(),
                                                         m_send_buffer.readable_bytes()),
                                     std::bind(&Connection::write_finish_handler,
                                               this->shared_from_this(),
                                               std::placeholders::_1,
                                               std::placeholders::_2));
        }
    }

    std::size_t completion_condition(const boost::system::error_code &ec, std::size_t bytes_transferred) {
        if (ec || !m_socket.is_open()) {
            m_timer.cancel();
            m_socket.cancel();
            m_status = utils::Status::ReadError();
            return 0;
        }
        restart_timer();
        const int check_ret = m_pkg_spliter(m_recv_buffer.read_index(), bytes_transferred);
        SNOW_LOG_TRACE("check ret {}", check_ret);
        if (check_ret > 0) {
            return 0;
        } else if (check_ret == 0) {
            return m_recv_buffer.writeable_bytes();
        } else {
            m_socket.close();
            m_timer.cancel();
            m_status = utils::Status::PkgCheckError();
            return 0;
        }
    }

    void read_finish_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) {
        if (ec || !m_socket.is_open()) {
            m_status = utils::Status::ReadError();
            if (m_socket.is_open()) {
                m_socket.close();
                //m_socket.cancel();
            }

            m_timer.cancel();
            return;
        }
        m_recv_buffer.increase_write_index(bytes_transferred);

        std::weak_ptr<Connection> conn(shared_from_this());
        m_request_dispatcher(m_recv_buffer.read_index(),
                             m_recv_buffer.readable_bytes(),
                             [this, conn](const char *const data, std::size_t size) {
                                 auto conn_ptr = conn.lock();
                                 if (conn_ptr) {
                                     send(data, size);
                                 }
                             });
        m_recv_buffer.adjuest();
        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_recv_buffer.write_index(), m_recv_buffer.writeable_bytes()),
                                std::bind(&Connection::completion_condition,
                                          this->shared_from_this(),
                                          std::placeholders::_1,
                                          std::placeholders::_2),
                                std::bind(&Connection::read_finish_handler,
                                          this->shared_from_this(),
                                          std::placeholders::_1,
                                          std::placeholders::_2));
    }

    void timeout_handler() {
        if (m_timer.expires_from_now() <= std::chrono::milliseconds(0)) {
            if (m_socket.is_open()) {
                try {
                    m_socket.cancel();
                    m_socket.close();
                } catch (std::exception e) {
                    SNOW_LOG_WARN("exception {}", e.what());
                }
            }
            m_status = utils::Status::Tiemout();
        }
    }

    void restart_timer() {
        m_timer.cancel();
        m_timer.expires_from_now(m_time_out);
        m_timer.async_wait(std::bind(&Connection::timeout_handler, this->shared_from_this()));
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::steady_timer m_timer;
    utils::request_dispatch_t m_request_dispatcher;
    utils::pkg_split_t m_pkg_spliter;
    FixedSizeBuffer<65536> m_send_buffer;
    FixedSizeBuffer<65536> m_recv_buffer;
    std::chrono::milliseconds m_time_out;
    utils::Status m_status;
};
}