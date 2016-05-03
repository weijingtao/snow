#ifndef _SNOW_SERVER_BASE_HPP
#define _SNOW_SERVER_BASE_HPP

#include <cstdint>
#include <array>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include "proxy.hpp"
#include "thread_poll.hpp"
#include "buffer.hpp"

namespace snow
{
    template <typename session_type>
    class server_base : public boost::noncopyable
    {
    public:
        typedef std::function<void(const buffer&)>      response_dispatch_type;

        server_base()
            : m_stop_flag(true),
              m_proxy(m_ios),
              m_thread_poll(10) {

        }

        virtual ~server_base() {
            stop();
        }

        void start() {
            m_stop_flag = false;
            m_thread_poll.start(std::bind(&server_base::run, this));
        }

        void stop() {
            m_stop_flag = true;
        }

    protected:
        virtual int init() {
            return 0;
        }

        virtual std::size_t pkg_check(const char* data, std::size_t len) = 0;


    private:
        void run() {
            SNOW_LOG_TRACE << "server runing" << std::endl;
            m_proxy.set_pkg_spliter(std::bind(&server_base::pkg_check, this, std::placeholders::_1, std::placeholders::_2));
            m_proxy.set_request_dispatcher(std::bind(&server_base::request_dispatch, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            while(!m_stop_flag) {
                m_ios.run();
            }
        }

        void request_dispatch(const char* req_data, std::size_t req_len, response_dispatch_type rsp_dispatcher) {
            SNOW_LOG_TRACE <<  "new request : " << req_data << std::endl;
            auto new_session = std::make_shared<session_type>(m_ios);
            new_session->set_response_dispatcher(std::move(rsp_dispatcher));
            new_session->start(req_data, req_len);
        }

    protected:
        bool                    m_stop_flag;
        boost::asio::io_service m_ios;
        proxy                   m_proxy;
        thread_poll             m_thread_poll;
    };
}

#endif //_SNOW_SERVER_BASE_HPP