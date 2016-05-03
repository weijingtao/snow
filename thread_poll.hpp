#ifndef _THREAD_POLL_HPP
#define _THREAD_POLL_HPP

#include <thread>
#include <vector>
#include <functional>

namespace snow
{
    class thread_poll
    {
    public:
        typedef std::function<void(void)> task_type;
        thread_poll(std::size_t thread_size)
            : m_thread_size(thread_size),
              m_threads(m_thread_size) {

        }

        void start(task_type task) {
            for(auto& thread : m_threads) {
                thread = std::thread(task);
            }
            for(auto& thread : m_threads) {
                thread.join();
            }
        }

        void stop() {
            for(auto& thread : m_threads) {
            }
        }



    private:
        std::size_t              m_thread_size;
        std::vector<std::thread> m_threads;
    };
}

#endif //_THREAD_POLL_HPP