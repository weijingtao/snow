#pragma once

#include <cstddef>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace snow {
    class WaitGroup {
    public:
        WaitGroup(boost::asio::io_service& ios, boost::asio::yield_context& yield, std::size_t count = 0)
                : m_deadline_timer{ios}
                , m_yield{yield}
                , m_count{count} {

        }

        ~WaitGroup() {
            m_deadline_timer.cancel();
        }

        WaitGroup(const WaitGroup&) = delete;

        void operator=(const WaitGroup&) = delete;

        void add(std::size_t num) { m_count += num; }

        void done() {
            m_count -= 1;
            if(m_count <= 0) {
                m_deadline_timer.cancel();
            }
        }

        void wait(std::chrono::milliseconds wait_time) {
            m_deadline_timer.expires_from_now(boost::posix_time::seconds(1));
            boost::system::error_code ec;
            m_deadline_timer.async_wait(m_yield[ec]);
        }


    private:
        boost::asio::deadline_timer m_deadline_timer;
        boost::asio::yield_context& m_yield;
        std::size_t m_count;
    };
}