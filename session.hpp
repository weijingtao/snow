#ifndef _SNOW_SESSION_HPP
#define _SNOW_SESSION_HPP

#include <memory>
#include <functional>
#include "session_base.hpp"
#include "buffer.hpp"
#include "request.hpp"
#include "response.hpp"

namespace snow
{

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