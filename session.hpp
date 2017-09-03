#pragma once

#include <memory>
#include <functional>
#include <chrono>
#include <type_traits>
#include <boost/optional.hpp>
#include "log/log.hpp"

namespace snow
{

    template <typename Req, typename Rsp>
    class session : public std::enable_shared_from_this<session<Req, Rsp>> {
    public:
        using request_t  = Req;
        using response_t = Rsp;
        using response_dispatch_t = std::function<void(boost::optional<response_t>&&)>;


        explicit session(boost::asio::io_service& ios)
                : m_ios{ios},
                  m_strand{ios},
                  m_start_time{std::chrono::steady_clock::now()} {
            SNOW_LOG_TRACE << "session construct" << std::endl;
            boost::asio::spawn(m_strand, [](boost::asio::yield_context yield) { SNOW_LOG_TRACE << "test" << std::endl; });
        }

        const std::chrono::steady_clock& get_start_time() const {
            return m_start_time;
        }

        void set_timeout(const std::chrono::milliseconds& timeout) {
            m_deadline = std::chrono::steady_clock::now() + timeout;
        }

        std::chrono::milliseconds get_time_left() const {
            auto now = std::chrono::steady_clock::now();
            if (m_deadline > now)
                return m_deadline - now;
            return std::chrono::milliseconds{0};
        }

        void set_response_dispatcher(response_dispatch_t response_dispatcher) {
            m_response_dispatcher = std::move(response_dispatcher);
        }

        bool start(const request_t& req) {
            SNOW_LOG_TRACE << "session start:" << req << std::endl;
            auto self(this->shared_from_this());
            boost::asio::spawn(m_strand,
                               [this, self, req](boost::asio::yield_context yield){
                                   set_yield_context_ptr(&yield);
                                   m_response_dispatcher(process(req));
                               });
            return true;
        }

        virtual boost::optional<response_t> process(const request_t& req) = 0;

    private:
        int set_yield_context_ptr(boost::asio::yield_context* yield_context_ptr) {
            m_yield_context_ptr = yield_context_ptr;
            return 0;
        }

        boost::asio::yield_context& get_yield_context() {
            return *m_yield_context_ptr;
        }


    private:
        using time_point = std::result_of<decltype(std::chrono::steady_clock::now)&(void)>::type;
        boost::asio::io_service &m_ios;
        boost::asio::strand m_strand;
        time_point m_start_time;
        time_point m_deadline;
        response_dispatch_t       m_response_dispatcher;
        boost::asio::yield_context*      m_yield_context_ptr;
    };
}