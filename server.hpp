#pragma once

#include <cstdint>
#include <array>
#include <mutex>
#include <string>
#include <boost/asio.hpp>
#include "proxy.hpp"
#include "thread_group.hpp"
#include "utils/buffer.hpp"
#include "utils/type.hpp"
#include "log/log.hpp"
#include "config.hpp"

namespace snow {
    template <typename SESSION>
    class server {
    public:
        using session_t  = SESSION;
        using request_t  = typename SESSION::request_t;
        using response_t = typename SESSION::response_t;
        using response_dispatch_t = typename SESSION::response_dispatch_t;

        server()
            : m_proxy(m_ios) {
        }

        virtual ~server() {
            stop();
        }

        server(const server&) = delete;
        server& operator=(const server&) = delete;

        void start() {
            std::call_once(m_start_flag, &server<session_t>::run, this);
        }

        void stop() {
            if (!m_ios.stopped())
                m_ios.stop();
        }

    protected:

        virtual int check(const char* data, std::size_t size) const = 0;

        virtual std::string encode(const response_t& req) const = 0;

        virtual request_t decode(const char* data, std::size_t size) const = 0;

    private:

        void run() {
            SNOW_LOG_TRACE("server runing");
            Config conf("../config.yaml");
            std::vector<std::tuple<std::string, std::string, uint16_t>> end_point_vec;
            for(auto &endpoint : conf.get_endpoints()) {
                auto pos1 = endpoint.find_first_of(':');
                auto pos2 = endpoint.find_first_of('/');
                if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 >= pos2) {
                    break;
                }
                end_point_vec.emplace_back(endpoint.substr(pos2+1),
                                           "",
                                           std::atoi(endpoint.substr(pos1+1, pos2 - pos1 -1).c_str()));
            }
            m_proxy.init(end_point_vec);
            using namespace std::placeholders;
            m_proxy.set_pkg_spliter(std::bind(&server<session_t>::check, this, _1, _2));
            m_proxy.set_request_dispatcher(std::bind(&server<session_t>::request_dispatch, this, _1, _2, _3));
            m_thread_group.start([this] {m_ios.run();}, conf.get_proc_num() );
            m_thread_group.join();
        }

        void request_dispatch(const char* req_data, std::size_t req_len, utils::response_dispatch_t rsp_dispatcher) {
            SNOW_LOG_TRACE("new request size {}", req_len);
            auto new_session = std::make_shared<session_t>(m_ios);
            new_session->set_response_dispatcher([this, rsp_dispatcher](boost::optional<response_t>&& rsp) {
                std::string str_rsp = encode(*rsp);
                SNOW_LOG_TRACE("rsp size {}", str_rsp.size());
                rsp_dispatcher(str_rsp.data(), str_rsp.size());
            });
            new_session->start(decode(req_data, req_len));
        }


    protected:
        std::once_flag          m_start_flag;
        boost::asio::io_service m_ios;
        proxy                   m_proxy;
        thread_group            m_thread_group;
    };
}