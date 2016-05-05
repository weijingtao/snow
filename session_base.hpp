#ifndef _SNOW_SESSION_BASSE_HPP
#define _SNOW_SESSION_BASSE_HPP

#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>

namespace snow
{
    class session_base : public std::enable_shared_from_this<session_base>
    {
    public:
        typedef std::function<void(const buffer&)>      response_dispatch_type;

        session_base(boost::asio::io_service& ios)
            : m_strand(ios),
              m_timer(ios),
              m_time_left(0) {

        }

        virtual ~session_base() {

        }

        void set_response_dispatcher(response_dispatch_type rsp_dispatcher) {
            m_rsp_dispatcher = std::move(rsp_dispatcher);
        }

        boost::asio::io_service::strand& get_strand() {
            return m_strand;
        }

        std::size_t get_time_left() const {
            return m_time_left;
        }

    protected:
        boost::asio::io_service::strand  m_strand;
        boost::asio::steady_timer        m_timer;
        response_dispatch_type           m_rsp_dispatcher;
        std::size_t                      m_time_left;
    };
}

#endif //_SNOW_SESSION_BASSE_HPP