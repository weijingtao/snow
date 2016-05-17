#ifndef _SNOW_SESSION_BASSE_HPP
#define _SNOW_SESSION_BASSE_HPP

#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include "request_base.hpp"
#include "response_base.hpp"

namespace snow
{
    class session_base : public std::enable_shared_from_this<session_base>
    {
    public:
        typedef std::function<void(const buffer&)>      response_dispatch_type;

        session_base(boost::asio::io_service& ios)
            : m_strand(ios),
              m_timer(ios),
              m_yield_context_ptr(nullptr),
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

        boost::asio::yield_context& get_yield_context() {
            return *m_yield_context_ptr;
        }

        //TODO
        void yield() {
//            m_yield_context_ptr->coro_.reset();
            m_coro = m_yield_context_ptr->coro_.lock();
            m_yield_context_ptr->ca_();
        }
        //TODO
        void resume() {
//            std::shared_ptr<boost::coroutines::push_coroutine<void>> coro = m_yield_context_ptr->coro_.lock();
            (*m_coro)();
//            (*(m_yield_context_ptr->coro_))();
        }


    protected:
        int set_yield_context_ptr(boost::asio::yield_context* yield_context_ptr) {
            m_yield_context_ptr = yield_context_ptr;
        }

    protected:
        boost::asio::io_service::strand  m_strand;
        boost::asio::steady_timer        m_timer;
        boost::asio::yield_context*      m_yield_context_ptr;
        response_dispatch_type           m_rsp_dispatcher;
        std::size_t                      m_time_left;

        std::shared_ptr<boost::coroutines::push_coroutine<void>> m_coro;
    };
}

#endif //_SNOW_SESSION_BASSE_HPP