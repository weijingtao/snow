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
    template <typename RequestType, typename ResponseType>
    class session : public std::enable_shared_from_this<session<RequestType, ResponseType>>
    {
    public:
        typedef RequestType                             request_type;
        typedef ResponseType                            response_type;
        typedef std::function<void(const buffer&)>      response_dispatch_type;

        explicit session(boost::asio::io_service& ios)
                : m_strand(ios),
                  m_timer(ios) {

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

        void set_response_dispatcher(response_dispatch_type rsp_dispatcher) {
            m_rsp_dispatcher = std::move(rsp_dispatcher);
        }

        virtual int process(const request_type& req, response_type* rsp, boost::asio::yield_context yield) = 0;


    protected:
        boost::asio::io_service::strand               m_strand;
        boost::asio::steady_timer                     m_timer;

        int       m_time_left;

    private:
        request_type           m_request;
        response_type          m_response;
        response_dispatch_type m_rsp_dispatcher;
    };
}



#endif //_SNOW_SESSION_HPP