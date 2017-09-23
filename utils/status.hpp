#pragma once

#include <string>
#include <ostream>
#include "spdlog/fmt/bundled/ostream.h"

namespace snow {
namespace utils {
class Status {
public:
    Status(int error_code, const std::string &error_msg)
            : m_error_code(error_code), m_error_msg(error_msg) {

    }

    Status() : Status(0, "ok") {}

    Status(const Status &rhs)
            : m_error_code(rhs.m_error_code),
              m_error_msg(rhs.m_error_msg) {

    }

    Status(Status &&rhs)
            : m_error_code(rhs.m_error_code), m_error_msg(std::move(rhs.m_error_msg)) {
        rhs.m_error_code = 0;
    }

    Status &operator=(const Status &rhs) {
        if (this != &rhs) {
            m_error_code = rhs.m_error_code;
            m_error_msg = rhs.m_error_msg;
        }
        return *this;
    }

    Status &operator=(Status &&rhs) {
        if (this != &rhs) {
            m_error_code = rhs.m_error_code;
            m_error_msg = std::move(rhs.m_error_msg);
            m_error_code = 0;
        }
        return *this;
    }

    bool operator==(const Status &rhs) {
        if (this == &rhs) {
            return true;
        }
        return m_error_code = rhs.m_error_code && m_error_msg == rhs.m_error_msg;
    }


    int get_error_code() const {
        return m_error_code;
    }

    const std::string &get_error_msg() const {
        return m_error_msg;
    }

    static Status OK() { return Status(0, "ok"); }

    static Status ConnectError() { return Status(-1, "connect error"); }

    static Status WriteError() { return Status(-2, "send error"); }

    static Status ReadError() { return Status(-3, "recv error"); }

    static Status Tiemout() { return Status(-4, "timeout"); }

    static Status PkgCheckError() { return Status(-5, "pkg check error"); }

    static Status AcceptError() { return Status(-6, "accept error"); }

    friend std::ostream &operator<<(std::ostream &os, const Status &error_code) {
        os << "{" << error_code.get_error_code()
           << "," << error_code.get_error_msg()
           << "}";
        return os;
    }

private:
    int m_error_code;
    std::string m_error_msg;
};
}
}