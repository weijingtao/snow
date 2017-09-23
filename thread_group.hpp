#pragma once

#include <thread>
#include <vector>
#include <functional>

namespace snow {
class thread_group {
public:
    typedef std::function<void(void)> task_type;

    thread_group() {

    }

    void start(task_type task, std::size_t thread_size) {
        for (std::size_t i = 0; i < thread_size; ++i) {
            m_threads.emplace_back(task);
        }
    }

    void join() {
        for (auto &thread : m_threads) {
            thread.join();
        }
    }


private:
    std::vector<std::thread> m_threads;
};
}