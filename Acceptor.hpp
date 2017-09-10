#pragma once

#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include "utils/status.hpp"

namespace snow {
    class Acceptor : public std::enable_shared_from_this<Acceptor> {
    public:
        typedef std::function<void (boost::asio::ip::tcp::socket& socket)> new_connect_call_back_t;

        Acceptor(boost::asio::io_service& ios, boost::asio::ip::tcp::endpoint endpoint)
            : m_acceptor(ios, endpoint)
            , m_accepted_socket(ios) {
            m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            // TODO 如果为linux，并且内核 > 3.9 开启reuse_port特性
            SNOW_LOG_TRACE("socket fd {}, listen addr {}",
                           m_acceptor.native(),
                           m_acceptor.local_endpoint());
        }

        ~Acceptor() {
            SNOW_LOG_TRACE("socket fd {}", m_acceptor.native());
        }

        void start() {
            if (m_acceptor.is_open()) {
                m_acceptor.async_accept(m_accepted_socket,
                                        std::bind(&Acceptor::accept_handler,
                                                  shared_from_this(),
                                                  std::placeholders::_1));
            }
        }

        void set_new_connecte_call_back(new_connect_call_back_t cb) {
            m_new_connect_call_back = std::move(cb);
        }

    private:
        void accept_handler(const boost::system::error_code& ec) {
            if(ec || !m_acceptor.is_open()) {
                m_status = utils::Status::AcceptError();
                return;
            }
            auto new_socket = std::move(m_accepted_socket);
            m_new_connect_call_back(new_socket);
            m_acceptor.async_accept(m_accepted_socket,
                                    std::bind(&Acceptor::accept_handler,
                                              shared_from_this(),
                                              std::placeholders::_1));
        }

    private:
        boost::asio::ip::tcp::acceptor  m_acceptor;
        boost::asio::ip::tcp::socket    m_accepted_socket;
        new_connect_call_back_t         m_new_connect_call_back;
        utils::Status                   m_status;
    };
}