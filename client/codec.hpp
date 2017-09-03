#pragma once

#include <string>
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
    class Codec {
    public:
        using request_t = REQ;
        using response_t = RSP;


        Codec() : m_error_code{0} {}

        virtual ~Codec() {}

        request_t& get_request() { return m_request; }

        const request_t& get_request() const { return m_request; }

        response_t& get_response() { return m_response; }

        const response_t& get_response() const { return m_response; }

        void set_error_code(int error_code) { m_error_code = error_code; }

        int get_error_code() const { return m_error_code; }

        void set_error_message(const std::string& error_message) { m_error_message = error_message; }

        const std::string& get_error_message() const { return m_error_message; }

        void set_error(int error_code, const std::string& error_messge) {
            m_error_code    = error_code;
            m_error_message = error_messge;
        }

        explicit operator bool() { return 0 == m_error_code; }

        virtual int check(const char* data, std::size_t size) const = 0;

        virtual bool encode(FixedSizeBuffer<4096>& buffer) const = 0;

        virtual bool decode(FixedSizeBuffer<4096>& buffer) = 0;

        virtual std::string get_dest_addr() const = 0;

    protected:
        request_t   m_request;
        response_t  m_response;
        std::string m_error_message;
        int         m_error_code;
    };
}