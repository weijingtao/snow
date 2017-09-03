#pragma once

#include <string>
#include "client/detail/codec_base.hpp"
#include "utils/fixed_size_buffer.hpp"

namespace snow {
    enum ErrorCode {
        OK = 0,
        Check_ERROR = 1,
        ENCODE_ERROR,
        DECODE_ERROR,
        SEND_ERROR,
        RECV_ERROR
    };

    template <typename REQ, typename RSP>
    class Codec : public detail::CodecBase {
    public:
        using request_t = REQ;
        using response_t = RSP;


        Codec() = default;

        virtual ~Codec() = default;

        request_t& get_request() { return m_request; }

        const request_t& get_request() const { return m_request; }

        response_t& get_response() { return m_response; }

        const response_t& get_response() const { return m_response; }

    protected:
        request_t   m_request;
        response_t  m_response;
    };
}