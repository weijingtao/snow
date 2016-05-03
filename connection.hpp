#ifndef _SNOW_CONNECTION_HPP
#define _SNOW_CONNECTION_HPP

#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include "buffer.hpp"
#include "log.hpp"

namespace snow
{
    class connection : public std::enable_shared_from_this<connection>
    {
    public:
        typedef std::function<void(const buffer&)>                                                  response_dispatch_type;
        typedef std::function<void(const char*, std::size_t, response_dispatch_type)>               request_dispatch_type;
        typedef std::function<std::size_t(const char*, std::size_t)>                                pkg_split_type;

        connection(boost::asio::ip::tcp::socket& socket,
                   request_dispatch_type request_dispatcher,
                   pkg_split_type pkg_spliter,
                   std::size_t time_out)
            : m_socket(std::move(socket)),
              m_timer(m_socket.get_io_service()),
              m_recv_strand(m_socket.get_io_service()),
              m_send_strand(m_socket.get_io_service()),
              m_request_dispatcher(std::move(request_dispatcher)),
              m_pkg_spliter(std::move(pkg_spliter)),
              m_recv_buffer(4096),
              m_time_out(time_out) {
            SNOW_LOG_TRACE << "connection construct socket[" << m_socket.native() << "]["
                           << m_socket.local_endpoint() << "<->" << m_socket.remote_endpoint() << "] "
                           << "timeout[" << m_time_out << "]" << std::endl;
        }

        ~connection() {
            SNOW_LOG_TRACE << "connection destruct socket" << std::endl;
        }

        void start() {
            auto self(shared_from_this());
            m_timer.expires_from_now(std::chrono::seconds(m_time_out));
            boost::asio::spawn(m_recv_strand, [this, self](boost::asio::yield_context yield) {
                try {
                    std::size_t n_read  = 0;
                    while (m_socket.is_open()) {
                        std::size_t pkg_len = 0;
                        do {
                            m_recv_buffer.ensure_writeable_bytes(RECV_BUFFER_MIN_WRITEABLE_BYTES);
                            std::size_t n = m_socket.async_read_some(boost::asio::buffer(m_recv_buffer.write_index(), m_recv_buffer.writeable_bytes()), yield);
                            SNOW_LOG_TRACE << "socket[" << m_socket.native() << "] " << "read " << n << " bytes" << std::endl;
                            if(n > 0) {
                                m_timer.expires_from_now(std::chrono::seconds(m_time_out));
                                m_recv_buffer.increase_write_index(n);
                                SNOW_LOG_TRACE << "socket[" << m_socket.native() << "] readable[" << m_recv_buffer.readable_bytes() << "]" << std::endl;
                            } else {
                                SNOW_LOG_TRACE << "socket read error" << std::endl;
                                m_socket.close();
                                m_timer.cancel();
                                break;
                            }
                            do {
                                pkg_len = m_pkg_spliter(m_recv_buffer.read_index(), m_recv_buffer.readable_bytes());
                                SNOW_LOG_TRACE << "pkg_len : " << pkg_len << std::endl;
                                if(pkg_len > 0) {
                                    std::string request_data(m_recv_buffer.read_index(), pkg_len);
                                    m_recv_buffer.increase_read_index(pkg_len);
                                    std::unique_ptr<request> new_request(new request(request_data));
                                    if(m_request_dispatcher) {
                                        m_request_dispatcher(std::move(new_request), std::bind(&connection::send, self, std::placeholders::_1));
                                    }
                                } else if(pkg_len < 0) {
                                    SNOW_LOG_TRACE << "socket read error" << std::endl;
                                    m_socket.close();
                                    m_timer.cancel();
                                }
                            } while(pkg_len > 0);
                        } while(0 == pkg_len);
                    }
                } catch (std::exception& e) {
                    m_socket.close();
                    m_timer.cancel();
                }
            });

            boost::asio::spawn(m_recv_strand, [this, self](boost::asio::yield_context yield) {
                while (m_socket.is_open()) {
                    boost::system::error_code ignored_ec;
                    m_timer.async_wait(yield[ignored_ec]);
                    if (m_timer.expires_from_now() <= std::chrono::seconds(0)) {
                        m_socket.close();
                    }
                }
            });
        }

    private:
        int send(const buffer& rsp) {
            SNOW_LOG_TRACE << "send : " << rsp.readable_bytes() << std::endl;
            auto self(shared_from_this());
            boost::asio::spawn(m_send_strand, [this, self, &rsp](boost::asio::yield_context yield) {
                auto response(std::move(const_cast<buffer&>(rsp)));
                while (m_socket.is_open() && response.readable_bytes() > 0) {
                    boost::system::error_code ignored_ec;
                    std::size_t n = m_socket.async_write_some(boost::asio::buffer(response.read_index(), response.readable_bytes()), yield);
                    if(n > 0) {
                        response.increase_read_index(n);
                    }
                    SNOW_LOG_TRACE << "socket[" << m_socket.native() << "] " << "write " << n << " bytes" << std::endl;
                    SNOW_LOG_TRACE << "response len[" << response->get_data().size() << "]" << "data[" << response->get_data() << "]" << std::endl;
                }
            });
            return 0;
        }

    private:
        static const std::size_t                       RECV_BUFFER_MIN_WRITEABLE_BYTES = 512;
        boost::asio::ip::tcp::socket                   m_socket;
        boost::asio::steady_timer                      m_timer;
        boost::asio::io_service::strand                m_recv_strand;
        boost::asio::io_service::strand                m_send_strand;
        request_dispatch_type                          m_request_dispatcher;
        pkg_split_type                                 m_pkg_spliter;
        buffer                                         m_recv_buffer;
        std::size_t                                    m_time_out;
    };
}


#endif //_SNOW_CONNECTION_HPP