#ifndef _SNOW_SESSION_HPP
#define _SNOW_SESSION_HPP

#include <cstdint>
#include <memory>
#include <array>
#include <functional>
#include <string>
#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include "buffer.hpp"
#include "request.hpp"
#include "response.hpp"

namespace snow
{
    class session_base : public std::enable_shared_from_this<session_base>
    {
    public:
        typedef std::function<void(const buffer&)>      response_dispatch_type;

        session_base(boost::asio::io_service& ios)
             : m_strand(ios),
               m_timer(ios),
               m_time_left(0) {

        }

        virtual ~session_base() {

        }

        void set_response_dispatcher(response_dispatch_type rsp_dispatcher) {
            m_rsp_dispatcher = std::move(rsp_dispatcher);
        }

        boost::asio::io_service::strand& get_strand() {
            return m_strand;
        }

        std::size_t get_time_left() const {
            return m_time_left;
        }

    protected:
        boost::asio::io_service::strand  m_strand;
        boost::asio::steady_timer        m_timer;
        response_dispatch_type           m_rsp_dispatcher;
        std::size_t                      m_time_left;
    };
    template <typename RequestType, typename ResponseType>
    class session : public session_base
    {
    public:
        typedef RequestType                             request_type;
        typedef ResponseType                            response_type;


        explicit session(boost::asio::io_service& ios)
                : session_base(ios) {

        }

        void start(const char* req_data, std::size_t req_len) {
            if(m_request.parse_from_array(req_data, req_len)) {
                auto self(this->shared_from_this());
                boost::asio::spawn(m_strand,
                               [this, self](boost::asio::yield_context yield){
                                   if(0 == process(m_request, &m_response, yield)) {
                                       buffer rsp_buffer;
                                       if(m_response.serialize_to_buffer(&rsp_buffer)) {
                                           m_rsp_dispatcher(rsp_buffer);
                                       }
                                   }
                               });
            }
        }

        virtual int process(const request_type& req, response_type* rsp, boost::asio::yield_context yield) = 0;


    private:
        request_type           m_request;
        response_type          m_response;
    };
}



#endif //_SNOW_SESSION_HPP