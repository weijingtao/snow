#ifndef _SNOW_WORKER_POLL_HPP
#define _SNOW_WORKER_POLL_HPP

#include <cstdint>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "log.hpp"

namespace snow
{
    class worker_poll : public boost::noncopyable
    {
        using io_service = boost::asio::io_service;
    public:

        static worker_poll& instance() {
            static worker_poll the_worker_poll(1);
            return the_worker_poll;
        }

        void start() {
            for (std::size_t i = 0; i < m_io_service_size; ++i) {
                std::unique_ptr<io_service> ios(new io_service);
                std::unique_ptr<std::thread> new_thread(new std::thread([&ios] { ios->run(); }));
                m_io_services.push_back(std::move(ios));
                m_threads.push_back(std::move(new_thread));
            }
            for (auto& thread : m_threads) {
                thread->join();
            }
        }

        void stop() {

        }

        // call onle from proxy thread, not thread safe
        io_service& get_io_service() {
//            std::atomic_compare_exchange_weak(&m_next_index, &m_io_service_size, std::size_t(0));
            SNOW_LOG_TRACE << "io service count : " << m_io_services.size() << std::endl;
            SNOW_LOG_TRACE << "next index --" << m_next_index << std::endl;
            if(m_next_index >= m_io_services.size())
                m_next_index = 0;
            SNOW_LOG_TRACE << "next index ++" << m_next_index << std::endl;
            return *m_io_services.at(m_next_index++);
        }

        void set_io_service_count(std::size_t io_service_size) {
            m_io_service_size = io_service_size;
        }

    private:
        worker_poll(std::size_t io_service_size)
            : m_io_service_size(io_service_size),
              m_next_index(0){
        }

    private:
        io_service                                m_ios;
        std::size_t                               m_io_service_size;
        std::vector<std::unique_ptr<io_service>>  m_io_services;
        std::atomic_size_t                        m_next_index;
        std::vector<std::unique_ptr<std::thread>> m_threads;
    };
}

#endif //_SNOW_WORKER_POLL_HPP