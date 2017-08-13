#pragma once

#include <cstdint>
#include <array>
#include <mutex>
#include <boost/asio.hpp>
#include "proxy.hpp"
#include "thread_group.hpp"
#include "buffer.hpp"

namespace snow
{
    template <typename SESSION>
    class server {
    public:
        using session_t  = SESSION;
        using request_t  = typename SESSION::request_t;
        using response_t = typename SESSION::response_t;
        using codec_t    = typename SESSION::codec_t;
        using response_dispatch_t = typename SESSION::response_dispatch_t;

        server()
            : m_proxy(m_ios),
              m_thread_poll(1) {

        }

        virtual ~server() {
            stop();
        }

        void start() {
            std::call_once(m_start_flag, &server<session_t>::run, this);
        }

        void stop() {
            if (!m_ios.stopped())
                m_ios.stop();
        }

    protected:
        virtual int init() {
            return 0;
        }

    private:
        void run() {
            SNOW_LOG_TRACE << "server runing" << std::endl;
            m_proxy.set_pkg_spliter(std::bind(&codec_t::check, std::ref(m_codec), std::placeholders::_1, std::placeholders::_2));
            m_proxy.set_request_dispatcher(std::bind(&server<session_t>::request_dispatch, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            m_ios.run();
        }

        void request_dispatch(const char* req_data, std::size_t req_len, typename proxy::response_dispatch_type rsp_dispatcher) {
            SNOW_LOG_TRACE <<  "new request : " << req_data << std::endl;
            auto new_session = std::make_shared<session_t>(m_ios);
            new_session->set_response_dispatcher([this, rsp_dispatcher](std::optional<response_t>&& rsp) {
                std::string str_rsp = m_codec.encode(*rsp);
                buffer b;
                b.append(str_rsp.data(), str_rsp.size());
                rsp_dispatcher(b);
            });
            new_session->start(std::string(req_data, req_len));
        }

    private:
        server(const server&) = delete;
        server& operator=(const server&) = delete;

    protected:
        std::once_flag          m_start_flag;
        boost::asio::io_service m_ios;
        proxy                   m_proxy;
        thread_group            m_thread_poll;
        codec_t                 m_codec;
    };
}