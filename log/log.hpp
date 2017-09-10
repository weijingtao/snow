#pragma once

#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/ostream.h"

namespace snow {
    class Logger {
    public:
        static std::shared_ptr<spdlog::logger> get() {
            static std::shared_ptr<spdlog::logger> logger
                    = spdlog::basic_logger_mt("snow", "./server1.txt", true);
            logger->set_level(spdlog::level::trace);
            return logger;
        }
    };
}

#define SNOW_LOG_TRACE   snow::Logger::get()->trace
#define SNOW_LOG_DEBUG   snow::Logger::get()->debug
#define SNOW_LOG_INFO    snow::Logger::get()->info
#define SNOW_LOG_WARN    snow::Logger::get()->warn
#define SNOW_LOG_ERROR   snow::Logger::get()->error
#define SNOW_LOG_FATAL   snow::Logger::get()->critical
