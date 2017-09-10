#pragma once

#include <functional>

namespace snow {
    namespace utils {
        using response_dispatch_t = std::function<void(const char*, std::size_t)>;
        using request_dispatch_t  = std::function<void(const char*, std::size_t, response_dispatch_t)>;
        using pkg_split_t         = std::function<int (const char*, std::size_t)>;
    }
}