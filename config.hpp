#pragma once

//#include <sys/sysinfo.h>
#include <cstdint>
#include <exception>
#include <algorithm>
#include <string>
#include <tuple>
#include <vector>
#include <locale>
#include "yaml-cpp/yaml.h"
#include "log/log.hpp"

namespace snow {
template<typename T = int>
class config {
public:
    config(const std::string &conf_file_name)
            : m_proc_num(2/*::get_nprocs()*/),
              m_connection_timeout(DEFAULT_CONNECTION_TIMEOUT),
              m_max_connecction(DEFAULT_MAX_CONNECTION),
              m_max_request_per_second(DEFAULT_MAX_REQUEST_PER_SECOND),
              m_log_level(DEFAULT_LOG_LEVEL),
              m_log_format(DEFAULT_LOG_FORMATE) {
        init(conf_file_name);
    }

    int get_proc_num() const {
        return m_proc_num;
    }

    int get_connection_timeout() const {
        return m_connection_timeout;
    }

    int get_max_connection() const {
        return m_max_connecction;
    }

    int get_max_request_per_second() const {
        return m_max_request_per_second;
    }

    int get_log_level() const {
        return m_log_level;
    }

    const std::string &get_log_format() const {
        return m_log_format;
    }

    const std::vector<std::string> &get_endpoints() const {
        return m_endpoints;
    }

private:

    config(const config &) = delete;

    void operator=(const config &) = delete;

    bool is_log_format_valid(const std::string &log_format) const {
        return true;
    }

    int init(const std::string &conf_file_name) {
        try {
            YAML::Node config = YAML::LoadFile(conf_file_name);
            if (config["interface"] && config["interface"].IsSequence()) {
                for (auto it = config["interface"].begin(); it != config["interface"].end(); ++it) {
                    m_endpoints.emplace_back(it->as<std::string>());
                }
            }

            if (config["service"] && config["service"].IsMap()) {
                if (config["service"]["procnum"]) {
                    m_proc_num = std::max(m_proc_num, config["service"]["procnum"].as<int>());
                }
                if (config["service"]["log"] && config["service"]["log"].IsMap()) {
                    if (config["service"]["log"]["level"]) {
                        m_log_level = std::max(config::MIN_LOG_LEVEL, config["service"]["log"]["level"].as<int>());
                        m_log_level = std::min(m_log_level, config["service"]["log"]["level"].as<int>());
                    }
                    if (config["service"]["log"]["format"] &&
                        is_log_format_valid(config["service"]["log"]["format"].as<std::string>())) {
                        m_log_format = config["service"]["log"]["format"].as<std::string>();
                    }
                }
            }

            if (config["limits"] && config["limits"].IsMap()) {
                if (config["limits"]["max_connection"]) {
                    m_max_connecction = std::max(MIN_CONNECTION, config["limits"]["max_connection"].as<int>());
                }
                if (config["limits"]["connection_timeout"]) {
                    m_connection_timeout = std::max(MIN_CONNECTION_TIMEOUT,
                                                    config["limits"]["connection_timeout"].as<int>());
                }
                if (config["limits"]["tcp_send_buf"]) {
                    std::cout << config["limits"]["tcp_send_buf"].as<int>() << std::endl;
                }
                if (config["limits"]["tcp_recv_buf"]) {
                    std::cout << config["limits"]["tcp_recv_buf"].as<int>() << std::endl;
                }
            }
            return 0;
        } catch (std::exception &e) {
            SNOW_LOG_FATAL("config init failed : {}", e.what());
            return -1;
        }
    }


    static const int MIN_PROC_NUM;
    static const int DEFAULT_CONNECTION_TIMEOUT;
    static const int MIN_CONNECTION_TIMEOUT;
    static const int DEFAULT_MAX_CONNECTION;
    static const int MIN_CONNECTION;
    static const int DEFAULT_MAX_REQUEST_PER_SECOND;
    static const int DEFAULT_LOG_LEVEL;
    static const int MIN_LOG_LEVEL;
    static const int MAX_LOG_LEVEL;
    static const char *const DEFAULT_LOG_FORMATE;

private:
    int m_proc_num;
    int m_connection_timeout; //ms
    int m_max_connecction;
    int m_max_request_per_second;
    int m_log_level;
    std::string m_log_format;
    std::vector<std::string> m_endpoints;
};

template<typename T>
const int config<T>::MIN_PROC_NUM = 1;

template<typename T>
const int config<T>::DEFAULT_CONNECTION_TIMEOUT = 60 * 1000; //1min

template<typename T>
const int config<T>::MIN_CONNECTION_TIMEOUT = 1000; //1s

template<typename T>
const int config<T>::DEFAULT_MAX_CONNECTION = 100;

template<typename T>
const int config<T>::MIN_CONNECTION = 1;

template<typename T>
const int config<T>::DEFAULT_MAX_REQUEST_PER_SECOND = 1000;

template<typename T>
const int config<T>::DEFAULT_LOG_LEVEL = 3;

template<typename T>
const int config<T>::MIN_LOG_LEVEL = 0;

template<typename T>
const int config<T>::MAX_LOG_LEVEL = 6;

template<typename T>
const char *const config<T>::DEFAULT_LOG_FORMATE = "test";

using Config = config<void>;
}