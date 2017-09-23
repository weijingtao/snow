#pragma once

#include <cstdint>
#include <string>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <boost/any.hpp>
#include "seq_generator.hpp"


namespace snow {
class Context {
public:
    using time_point = std::chrono::steady_clock::time_point;
    using duration   = std::chrono::steady_clock::duration;

    Context(uint32_t id = SeqGenerator::instance().get())
            : m_id{id}, m_start_time{std::chrono::steady_clock::now()} {

    }

    ~Context() = default;

    Context(const Context &) = delete;

    void operator=(const Context &) = delete;

    template<typename T>
    void set(const std::string &name, T &&value) {
        m_values[name] = value;
    }

    template<typename T>
    void set(const std::string &name, const T &value) {
        m_values[name] = value;
    }

    template<typename T>
    const T &get(const std::string &name) const {
        return boost::any_cast<T &>(&m_values[name]);
    }

    bool has(const std::string &name) {
        return m_values.cend() != m_values.find(name);
    }

    void set_timeout(duration timeout) {
        m_deadline = m_start_time + timeout;
    }

    duration timecost() const {
        return std::chrono::steady_clock::now() - m_start_time;
    }

    duration timeleft() const {
        return m_deadline - std::chrono::steady_clock::now();
    }


private:
    uint32_t m_id;
    time_point m_start_time;
    time_point m_deadline;
    std::unordered_map<std::string, boost::any> m_values;
};
}