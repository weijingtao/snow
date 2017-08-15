#pragma once

#include <functional>
#include <boost/asio.hpp>

namespace snow
{
    class acceptor
    {
    public:
        typedef std::function<void (boost::asio::ip::tcp::socket& socket)> new_connect_call_back_t;

        acceptor(boost::asio::io_service& ios, boost::asio::ip::tcp::endpoint endpoint)
            : m_strand(ios),
              m_acceptor(ios, endpoint) {
            m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            SNOW_LOG_TRACE << "acceptor construct socket[" << m_acceptor.native() << "]" << std::endl;
        }

        //TODO
        /*~acceptor() {
            SNOW_LOG_TRACE << "acceptor destruct" << std::endl;
        }*/

        boost::asio::io_service::strand& strand() {
            return m_strand;
        }

        void operator()(boost::asio::yield_context yield) {
            while (m_acceptor.is_open()) {
                boost::system::error_code ec;
                boost::asio::ip::tcp::socket new_socket(m_strand.get_io_service());
                m_acceptor.async_accept(new_socket, yield[ec]);
                if (!ec) {
                    SNOW_LOG_TRACE << "accept new socket fd[" << new_socket.native() << "] " << new_socket.local_endpoint() << " <- " << new_socket.remote_endpoint() << std::endl;
                    if(m_new_connect_call_back) {
                        m_new_connect_call_back(new_socket);
                    }
                }
            }
        }

        void set_new_connecte_call_back(new_connect_call_back_t cb) {
            m_new_connect_call_back = std::move(cb);
        }

    private:
        boost::asio::io_service::strand m_strand;
        boost::asio::ip::tcp::acceptor  m_acceptor;
        new_connect_call_back_t         m_new_connect_call_back;
    };
}