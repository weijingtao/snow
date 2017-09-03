#pragma once

#include <cstddef>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace snow {
    class WaitGroup {
    public:
        using cancel_t = std::function<void(void)>;
        WaitGroup(boost::asio::io_service& ios, boost::asio::yield_context& yield)
                : m_deadline_timer{ios}
                , m_yield{yield}
                , m_seq{100} {

        }

        ~WaitGroup() {
            m_deadline_timer.cancel();
            cancel_all();
        }

        WaitGroup(const WaitGroup&) = delete;

        void operator=(const WaitGroup&) = delete;

        std::size_t add(cancel_t&& cancel) {
            auto id = ++m_seq;
            m_cancels[id] = cancel;
            return id;
        }

        void done(std::size_t id) {
            m_cancels.erase(id);
            if(m_cancels.empty()) {
                m_deadline_timer.cancel();
            }
        }

        void wait(std::chrono::milliseconds wait_time) {
            m_deadline_timer.expires_from_now(boost::posix_time::seconds(1));
            boost::system::error_code ec;
            m_deadline_timer.async_wait(m_yield[ec]);
            if (m_deadline_timer.expires_from_now() <= std::chrono::seconds(0)) {
                cancel_all();
            }
        }

    private:
        void cancel_all() {
            for(auto& it : m_cancels) {
                if(it.second) {
                    it.second();
                }
            }
        }


    private:
        boost::asio::deadline_timer               m_deadline_timer;
        boost::asio::yield_context&               m_yield;
        std::unordered_map<std::size_t, cancel_t> m_cancels;
        std::size_t                               m_seq;
    };
}